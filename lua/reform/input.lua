local M = {
	default = vim.ui.input,
	---@type reform.input.Config
	config = {
		window = vim.tbl_extend('force', {
			height = 1,
			row = -3,
		}, require('reform.util').winConfig),
		keymaps = {
			cancel = { '<Esc>', '<C-q>' },
			confirm = { '<CR>' },
			histPrev = { '<Up>', '<M-k>' },
			histNext = { '<Down>', '<M-j>' },
		},
	},
}

function M.override(opts, on_confirm)
	local default = opts.default or ''
	opts.prompt = opts.prompt and opts.prompt:gsub(': *$', '') or ''
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-input'
	local width = #opts.prompt > #default and #opts.prompt + 2 or #default

	M.config.window.col = -width / 2
	M.config.window.width = width + (#default > 10 and #default * 2 or 20)
	local win = require('reform.util').mkWin(buf, M.config.window, opts.prompt)
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
	for name, action in pairs {
		cancel = callback,
		confirm = function() callback(true) end,
		histPrev = function()
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
		histNext = function()
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
		for _, key in ipairs(M.config.keymaps[name]) do
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

---@param config reform.Overridable|reform.input.Config
function M.setup(config)
	if config then
		if type(config) == 'function' then
			vim.ui.input = config
		else
			vim.ui.input = M.override
			if type(config) == 'table' then M.config = vim.tbl_deep_extend('force', M.config, config) end
		end
	else
		vim.ui.input = M.default
	end
end

return M
