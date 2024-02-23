---@type reform.util
---@diagnostic disable-next-line: missing-fields
local M = {
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

function M.findCursorMatch(event, config, default)
	local line = vim.api.nvim_buf_get_lines(event.buf, event.line - 1, event.line, true)[1]
	local column = event.column

	if type(config) == 'function' then config = config(event) end
	for _, handler in ipairs(config) do
		if type(handler) == 'string' then handler = default[handler] end
		if handler.luapat then -- lua pattern
			local found = { line:find(handler.luapat, 1) }
			while found[1] do
				if (event.freeFrom or found[1] <= column) and column <= found[2] then
					local info = { from = found[1], to = found[2] }
					local i = 3
					while found[i] do
						info[i - 2] = found[i]
						i = i + 1
					end
					if handler.use(info[1], info, event) ~= false then return true end -- handler success
				end
				found = { line:find(handler.luapat, found[2] + 1) }
			end
		else -- vim regex
			local re = vim.regex(handler.vimre)
			local offset = 1
			local from, to = re:match_str(line)
			while from do
				from, to = from + offset, to + offset - 1
				if (event.freeFrom or from[1] <= column) and column <= to then
					local info = vim.fn.matchlist(line:sub(from, to), handler.vimre) -- includes \\0
					info.from = from
					info.to = to
					if handler.use(info[2], info, event) ~= false then return true end -- handler success
				end
				offset = to + 1
				from, to = re:match_str(line:sub(offset))
			end
		end
	end

	return false -- parsing failed
end

return M
