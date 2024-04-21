---@diagnostic disable: inject-field
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
				height = 1,
				row = -3,
			}, require('reform.util').win),
			select = vim.tbl_extend(
				'force',
				{ col = -2, row = 1, winhl = 'Id:Repeat,VarDelim:Delimiter' },
				require('reform.util').win
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
	local default = opts.default or ''
	opts.prompt = opts.prompt and opts.prompt:gsub(':? *$', '') or ''
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-input'
	local width = #opts.prompt > #default and #opts.prompt + 2 or #default

	M.config.win.input.col = -width / 2
	M.config.win.input.width = width + (#default > 10 and #default * 2 or 20)
	local win = require('reform.util').mkWin(buf, M.config.win.input, opts.prompt)
	vim.wo[win].cursorline = false
	vim.api.nvim_set_current_line(default)
	vim.cmd.startinsert { bang = true }

	local typed = default
	local last = vim.fn.histnr '@'
	local histpos = last + 1
	local histUpdate = true

	local function callback(confirmed)
		local text = vim.api.nvim_get_current_line()
		vim.api.nvim_win_close(win, true)
		if confirmed then
			if #text > 3 and not opts.prompt:match '[Nn]ame' then vim.fn.histadd('@', text) end
			on_confirm(text)
		else
			on_confirm(opts.cancelreturn == nil and '' or opts.cancelreturn)
		end
	end

	vim.api.nvim_create_autocmd('ModeChanged', {
		buffer = buf,
		callback = function(state)
			if state.match == 'i:n' then callback() end
		end,
	})

	for name, action in pairs {
		cancel = callback,

		confirm = function() callback(true) end,

		hist_prev = function()
			if histpos == last + 1 then typed = vim.api.nvim_get_current_line() end
			for i = histpos - 1, 1, -1 do
				local str = vim.fn.histget('@', i)
				if str:find(typed, 1, true) then
					histpos = i
					histUpdate = true
					vim.api.nvim_set_current_line(str)
					return
				end
			end
		end,

		hist_next = function()
			if histpos == last + 1 then return end
			for i = histpos + 1, last do
				local str = vim.fn.histget('@', i)
				if str:find(typed, 1, true) then
					histpos = i
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
				histpos = last + 1
			else
				histUpdate = false
			end
			if opts.highlight then opts.highlight(buf) end
		end,
	})
end

function M.override.reform.select(items, opts, on_choice)
	opts.prompt = opts.prompt and opts.prompt:gsub(': *$', '') or 'Select one of'
	opts.format_item = opts.format_item or tostring
	local callback
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-select'
	local width = #opts.prompt
	local lines = {}
	for i, item in ipairs(items) do
		lines[i] = '[' .. i .. '] ' .. opts.format_item(item)
		if #lines[i] > width then width = #lines[i] end
		vim.keymap.set('n', tostring(i), function() callback(i) end, { buffer = buf })
	end
	vim.api.nvim_buf_set_lines(buf, 0, -1, true, lines)
	vim.bo[buf].modifiable = false
	vim.cmd.stopinsert()

	M.config.win.select.height = #items
	M.config.win.select.width = width
	local win = require('reform.util').mkWin(buf, M.config.win.select, opts.prompt)
	vim.api.nvim_win_set_cursor(win, { 1, 1 })
	vim.api.nvim_create_autocmd('CursorMoved', {
		buffer = buf,
		callback = function()
			local cursor = vim.api.nvim_win_get_cursor(win)
			if cursor[2] ~= 1 then vim.api.nvim_win_set_cursor(win, { cursor[1], 1 }) end
		end,
	})
	vim.cmd.stopinsert()

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
end

return M
