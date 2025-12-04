---@diagnostic disable: inject-field
local util = require 'reform.util'

---@type reform.ui
---@diagnostic disable-next-line: missing-fields
local M = {
	override = {
		set = {
			input = function(fn) vim.ui.input = fn end,
			select = function(fn) vim.ui.select = fn end,
		},
		vim = { input = vim.ui.input, select = vim.ui.select },
		reform = {},
	},
	default_config = {
		override = { input = true, select = true },
		win = {
			input = vim.tbl_extend('force', {
				height = 2,
				row = -3,
			}, util.win),
			select = vim.tbl_extend(
				'force',
				{ col = -2, row = 1, winhl = 'Id:Repeat,VarDelim:Delimiter' },
				util.win
			),
		},
		input_mapping = { -- TODO: refactor to use standard mechanism, same for select
			cancel = { '<Esc>', '<C-q>' },
			confirm = { '<CR>' },
			hist_prev = { '<Up>', '<A-k>' },
			hist_next = { '<Down>', '<A-j>' },
		},
	},
}
M.config = M.default_config

function M.override.reform.input(opts, on_confirm)
	opts = opts or {}
	local origDir = vim.uv.cwd()
	vim.fn.chdir(opts.dir or origDir)

	local co -- support for providing input synchronously via coroutines
	if not on_confirm then
		co = coroutine.running()
		assert(co, 'run in coroutine.wrap()ped fn or provide callback')
		on_confirm = function(...) coroutine.resume(co, ...) end
	end

	local default = opts.default or ''
	local prompt = (opts.prompt or ''):gsub(':? *$', '')

	local buf = vim.api.nvim_create_buf(false, true)
	local width = math.max(#prompt + 2, #default)

	M.config.win.input.col = -width / 2
	M.config.win.input.width = width + math.max(#default * 2, 40)
	local win = util.mk_win(buf, M.config.win.input, prompt)
	vim.wo[win].cursorline = false
	vim.api.nvim_set_current_line(default)

	vim.bo[buf].filetype = 'ui-input'
	local nsName = 'vim.ui.input'
	local ns = vim.api.nvim_get_namespaces()[nsName] or vim.api.nvim_create_namespace(nsName)

	local typed = default
	local last = vim.fn.histnr '@'
	local histPos = last + 1
	local histUpdate = true

	local function callback(confirmed)
		vim.fn.chdir(origDir)

		local text = vim.api.nvim_get_current_line()
		vim.api.nvim_win_close(win, true)
		if confirmed then
			if #text > 3 and not prompt:match '[Nn]ame' then vim.fn.histadd('@', text) end
			on_confirm(text)
		else
			on_confirm(opts.cancelreturn == nil and '' or opts.cancelreturn)
		end
	end

	vim.cmd { cmd = 'startinsert', bang = true }
	vim.api.nvim_create_autocmd('ModeChanged', {
		callback = function(_) callback() end, -- cancel the window
		once = true,
		pattern = 'i:n',
	})

	for name, action in pairs {
		cancel = callback,

		confirm = function() callback(true) end,

		hist_prev = function()
			if histPos == last + 1 then typed = vim.api.nvim_get_current_line() end
			for i = histPos - 1, 1, -1 do
				local str = vim.fn.histget('@', i)
				if str:find(typed, 1, true) then
					histPos = i
					histUpdate = true
					vim.api.nvim_set_current_line(str)
					return
				end
			end
		end,

		hist_next = function()
			if histPos == last + 1 then return end
			for i = histPos + 1, last do
				local str = vim.fn.histget('@', i)
				if str:find(typed, 1, true) then
					histPos = i
					histUpdate = true
					vim.api.nvim_set_current_line(str)
					return
				end
			end
			vim.api.nvim_set_current_line(typed)
		end,
	} do
		for _, key in ipairs(M.config.input_mapping[name]) do
			vim.keymap.set('i', key, action, { buffer = buf })
		end
	end

	vim.api.nvim_create_autocmd('TextChangedI', {
		buffer = buf,
		callback = function()
			if not histUpdate then
				histPos = last + 1
			else
				histUpdate = false
			end
			if opts.highlight then
				for _, hi in ipairs(opts.highlight(vim.api.nvim_get_current_line())) do
					vim.hl.range(buf, ns, hi[3], { 0, hi[1] - 1 }, { 0, hi[2] })
				end
			end
		end,
	})

	if co then return coroutine.yield() end
end

function M.override.reform.select(items, opts, on_choice)
	opts = opts or {}
	local co
	if not on_choice then
		co = coroutine.running()
		assert(co, 'run in coroutine.wrap()ped fn or provide callback')
		on_choice = function(...) coroutine.resume(co, ...) end
	end

	local prompt = (opts.prompt or 'Select one of'):gsub(': *$', '')
	opts.format_item = opts.format_item or tostring
	local callback
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-select'
	local width = #prompt
	local lines = { [#items] = '' }
	for i, item in ipairs(items) do
		item = '[' .. i .. '] ' .. opts.format_item(item)
		if #item > width then width = #item end
		lines[i] = item
		vim.keymap.set('n', tostring(i), function() callback(i) end, { buffer = buf })
	end
	vim.api.nvim_buf_set_lines(buf, 0, -1, true, lines)
	vim.bo[buf].modifiable = false

	M.config.win.select.height = #items
	M.config.win.select.width = width
	local win = util.mk_win(buf, M.config.win.select, prompt)
	vim.api.nvim_win_set_cursor(win, { 1, 1 })
	vim.api.nvim_create_autocmd('CursorMoved', {
		buffer = buf,
		callback = function()
			local cursor = vim.api.nvim_win_get_cursor(win)
			if cursor[2] ~= 1 then vim.api.nvim_win_set_cursor(win, { cursor[1], 1 }) end
		end,
	})
	vim.cmd 'stopinsert'

	vim.cmd [[
		syn match Number /-\?\d\+/
		syn match Id /\d\+/ contained
		syn match Delimiter /^\[\d\+\] / contains=Id
		syn region String start=/"/ skip=/'/ end=/"/ contained
		syn region String start=/'/ skip=/"/ end=/'/ contained
		syn match Variable /\w\+/ contained
		syn region VarDelim start="`" end="`" contains=Variable
	]]

	callback = function(i)
		vim.api.nvim_win_close(win, true)
		on_choice(i and items[i], i)
	end
	vim.keymap.set('n', 'q', callback, { buffer = buf })
	vim.keymap.set('n', '<Esc>', callback, { buffer = buf })
	vim.keymap.set(
		'n',
		'<CR>',
		function() callback(vim.api.nvim_win_get_cursor(win)[1]) end,
		{ buffer = buf }
	)

	if co then return coroutine.yield() end
end

return M
