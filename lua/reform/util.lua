local M = {
	---@type reform.WinConfig
	winConfig = {
		title_pos = 'center',
		title = { { '[', 'FloatBorder' }, { '', 'FloatTitle' }, { ']', 'FloatBorder' } },
		relative = 'cursor',
		border = 'rounded',
	},
}
function M.mkWin(buf, opts, prompt)
	local mode = vim.api.nvim_get_mode().mode
	vim.bo[buf].buftype = 'nofile'
	vim.bo[buf].bufhidden = 'wipe'
	if opts.height < 1 then opts.height = 1 end
	if opts.width < 1 then opts.width = 1 end
	opts.title[2][1] = prompt ~= '' and prompt or '⋯'
	local winhl = opts.winhl or ''
	opts.winhl = nil
	local win = vim.api.nvim_open_win(buf, true, opts)
	opts.winhl = winhl
	vim.wo[win].winhighlight = 'Search:NONE,Pmenu:Normal,MatchParen:NONE,' .. winhl
	vim.wo[win].number = false
	vim.wo[win].relativenumber = false
	vim.wo[win].cursorcolumn = false
	vim.api.nvim_create_autocmd('BufLeave', {
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
