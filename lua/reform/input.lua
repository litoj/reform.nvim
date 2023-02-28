local M = {default = vim.ui.input}

function M.override(opts, on_confirm)
	local mode = vim.api.nvim_get_mode().mode
	local default = opts.default or ""
	local prompt = opts.prompt and opts.prompt:gsub(": *$", "") or ""
	local buf = vim.api.nvim_create_buf(false, true)
	local width = #prompt > #default and #prompt + 2 or #default
	local win = require'reform.util'.mkWin(buf, {
		row = -3, -- render window at `row` lines from cursor position
		col = -width / 2,
		width = width + (#default > 25 and 15 or 5),
		height = 1,
	}, prompt)
	vim.wo[win].cursorline = false
	vim.api.nvim_buf_set_lines(buf, 0, -1, true, {default})
	vim.cmd.startinsert {bang = true}

	local function callback(confirmed)
		if mode ~= "i" then vim.api.nvim_input("<right>") end
		local text = confirmed and vim.api.nvim_buf_get_lines(buf, 0, -1, true)[1]
		vim.api.nvim_win_close(win, true)
		if confirmed then
			on_confirm(text)
		else
			on_confirm(opts.cancelreturn == nil and "" or opts.cancelreturn)
		end
	end
	vim.keymap.set("i", "<C-q>", callback, {buffer = buf})
	vim.keymap.set("i", "<Esc>", callback, {buffer = buf})
	vim.keymap.set("i", "<CR>", function() callback(true) end, {buffer = buf})
end

return function(config)
	if config then
		vim.ui.input = type(config) == "function" and config or M.override
	else
		vim.ui.input = M.default
	end
	return M
end
