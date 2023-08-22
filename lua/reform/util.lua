local M = {}
function M.mkWin(buf, opts, prompt)
	local mode = vim.api.nvim_get_mode().mode
	vim.bo[buf].buftype = 'nofile'
	vim.bo[buf].bufhidden = 'wipe'
	if opts.height < 1 then opts.height = 1 end
	if opts.width < 1 then opts.width = 1 end
	if prompt ~= '' then
		opts.title[2][1] = prompt
	else
		opts.title = nil
	end
	local win = vim.api.nvim_open_win(buf, true, opts)
	vim.wo[win].winhighlight = 'Search:NONE,Pmenu:Normal,MatchParen:NONE'
	vim.wo[win].number = false
	vim.wo[win].cursorcolumn = false
	vim.api.nvim_create_autocmd('BufWinLeave', {
		buffer = buf,
		callback = function()
			if mode == 'i' then
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
