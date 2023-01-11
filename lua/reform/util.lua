local M = {}
function M.mkWin(buf, opts, prompt)
	local mode = vim.api.nvim_get_mode().mode
	vim.bo[buf].buftype = "nofile"
	vim.bo[buf].bufhidden = "wipe"
	vim.bo[buf].filetype = "Select"
	opts = vim.tbl_extend("force", {relative = "cursor", border = "rounded"}, opts)

	local promptWin = nil
	if prompt ~= "" then
		prompt = "[" .. prompt:gsub(" $", "") .. "]"
		if opts.width < #prompt then
			opts.width = #prompt
		else
			if (opts.width - #prompt) % 2 ~= 0 then opts.width = opts.width + 1 end
		end

		local promptBuf = vim.api.nvim_create_buf(false, true)
		vim.bo[promptBuf].buftype = "nofile"
		vim.bo[promptBuf].bufhidden = "wipe"
		vim.api.nvim_buf_set_lines(promptBuf, 0, -1, true, {prompt})
		vim.bo[promptBuf].modifiable = false

		promptWin = vim.api.nvim_open_win(promptBuf, false, {
			relative = "cursor",
			row = opts.row,
			col = opts.col + (opts.width - #prompt) / 2 + 1,
			width = #prompt,
			height = 1,
			border = "none",
			zindex = 999,
		})
		vim.wo[promptWin].number = false
		vim.wo[promptWin].cursorline = false
		vim.wo[promptWin].cursorcolumn = false
		vim.wo[promptWin].winhighlight = "Search:NONE,Normal:FloatBorder"
		vim.api.nvim_buf_add_highlight(promptBuf, -1, "FloatTitle", 0, 1, #prompt - 1)
	end

	local win = vim.api.nvim_open_win(buf, true, opts)
	vim.wo[win].winhighlight = "Search:NONE,Pmenu:Normal,MatchParen:NONE"
	vim.wo[win].number = false
	vim.wo[win].cursorcolumn = false
	vim.api.nvim_create_autocmd("BufLeave", {
		buffer = buf,
		once = true,
		callback = function()
			if promptWin then vim.api.nvim_win_close(promptWin, true) end
			if mode == "i" then
				vim.cmd.startinsert()
			else
				vim.cmd.stopinsert()
			end
			vim.api.nvim_win_close(win, true)
		end,
	})

	return win
end

return M
