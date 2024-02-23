---@type reform.util
---@diagnostic disable-next-line: missing-fields
local M = {
	winConfig = {
		title_pos = 'center',
		title = { { '[', 'FloatBorder' }, { '', 'FloatTitle' }, { ']', 'FloatBorder' } },
		relative = 'cursor',
		border = 'rounded',
	},
	debug = false,
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

function M.findMatch(event, matchers, default, filter)
	if event.opts.sorting then filter = event.opts.sorting end
	local line = vim.api.nvim_buf_get_lines(event.buf, event.line - 1, event.line, true)[1]
	local column = event.column

	local function formatFind(from, to, ...) return { from = from, to = to, ... } end
	local function iter(matcher)
		local ret
		if matcher.luapat then
			local pat = matcher.luapat
			ret = function(_, from)
				local ret = formatFind(line:find(pat, from))
				if ret[1] then return ret.to + 1, ret end
			end
		else
			local re = vim.regex(matcher.vimre)
			ret = function(_, _from)
				local from, to = re:match_str(line:sub(_from))
				if not from then return end
				from, to = from + _from, to + _from - 1
				local ret = vim.fn.matchlist(line:sub(from, to), matcher.vimre) -- includes entire match
				ret.from = from
				ret.to = to
				return to + 1, ret
			end
		end
		return ret, nil, 1
	end

	if type(filter) ~= 'function' then
		local prio = filter
		filter = function(order, matcher, match)
			return prio.order * order
				+ prio.matcher * (matcher.weight or 0)
				+ prio.offset * (match.from - column)
				+ prio.length * #match[1]
		end
	end
	local order = {}

	if type(matchers) == 'function' then matchers = matchers(event) end
	for i, matcher in ipairs(matchers) do
		if type(matcher) == 'string' then matcher = default[matcher] end
		for _, match in iter(matcher) do
			if column <= match.to then
				if match.from <= column then -- cursor within match
					if matcher.use(match[1], match, event) ~= false then
						if event.opts.setCol then
							local col = event.opts.setCol(event, match)
							if col then vim.api.nvim_win_set_cursor(0, { event.line, col }) end
						end
						return true
					end
				else
					order[#order + 1] = { filter(i, matcher, match), matcher, match }
				end
			end
		end
	end

	table.sort(order, function(a, b) return a[1] < b[1] end)
	if M.debug then vim.notify('reform.util.findMatch order: ' .. vim.inspect(order)) end
	for _, pair in ipairs(order) do
		if pair[2].use(pair[3][1], pair[3], event) ~= false then
			if event.opts.setCol then
				local col = event.opts.setCol(event, pair[3])
				if col then vim.api.nvim_win_set_cursor(0, { event.line, col }) end
			end
			return true
		end
	end

	return false -- parsing failed
end

return M
