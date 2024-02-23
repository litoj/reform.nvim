local M = {
	defaults = {
		unknown = 'definition',
		mappings = {
			{ { 'n', 'i' }, '<A-a>', { action = 'inc', freeFrom = true } },
			{ { 'n', 'i' }, '<A-A>', { action = 'dec', freeFrom = true } },
			{ { 'n', 'i' }, '<A-C-a>', { action = 'tgl' } },
		},
	},
}
function M.replace(match, event, with)
	local line = vim.api.nvim_buf_get_lines(event.buf, event.line - 1, event.line, true)[1]
	line = line:sub(1, match.from - 1) .. with .. line:sub(match.to + 1)
	vim.api.nvim_buf_set_lines(event.buf, event.line - 1, event.line, true, { line })
end
function M.genSeqHandler(seq, matcher)
	local handler = {
		luapat = matcher and matcher.luapat,
		vimre = matcher and matcher.vimre,
		use = function(_, match, ev)
			local dir = ev.action == 'dec' and -1 or (ev.action == 'inc' and 1 or #seq / 2)
			M.replace(match, ev, seq[(seq[match[1]] - 1 + dir) % #seq + 1])
		end,
	}
	local pat = {}
	if matcher then
		for i, v in ipairs(seq) do
			seq[v] = i
		end
	else
		if #seq[1] == 1 then
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = (v == ']' or v == '%') and '%' .. v or v
			end
			handler.luapat = '([' .. table.concat(pat, '') .. '])'
		else
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = v:gsub('[\\.*[%]]', '\\%0')
			end
			handler.vimre = table.concat(pat, '\\|')
		end
	end
	return handler
end
M.handlers = {
	answer = M.genSeqHandler({ 'yes', 'no' }, { vimre = [[\<\(yes\|no\)\>]] }),
	bool = {
		vimre = [[[Tt]rue\|[Ff]alse]],
		use = function(_, match, event)
			if match[1]:byte(2) == 114 then
				M.replace(match, event, match[1]:byte() == 84 and 'False' or 'false')
			else
				M.replace(match, event, match[1]:byte() == 70 and 'True' or 'true')
			end
		end,
	},
	comparator = M.genSeqHandler { '>', '<' },
	direction = M.genSeqHandler(
		{ 'up', 'north', 'east', 'down', 'south', 'west' },
		{ vimre = [[\<\(up\|north\|east\|down\|south\|west\)\>]] }
	),
	int = {
		vimre = [[-\?\d\+]],
		use = function(_, match, event)
			local num = tonumber(match[1])
			if event.action == 'tgl' then
				M.replace(match, event, tostring(-num))
			else
				M.replace(match, event, tostring(num + (event.action == 'inc' and 1 or -1)))
			end
		end,
	},
	logic = M.genSeqHandler({ 'and', '&', 'or', '|' }, { vimre = [[[&|]\|\<and\>\|\<or\>]] }),
	sign = M.genSeqHandler { '+', '-' },
	state = {
		vimre = [[enabled\?\|disabled\?]],
		use = function(_, match, event)
			if match[1]:byte() == 101 then
				M.replace(match, event, match[1]:byte(-1) == 100 and 'disabled' or 'disable')
			else
				M.replace(match, event, match[1]:byte(-1) == 100 and 'enabled' or 'enable')
			end
		end,
	},
	toggle = M.genSeqHandler({ 'on', 'off' }, { vimre = [[\<o(n\|ff)\>]] }),
}

function M.handle(ev)
	if require('reform.util').findCursorMatch(ev, M.config.handlers, M.handlers) then return end
	vim.notify 'No toggleable found'
end

function M.genBind(ev)
	ev.buf = ev.buf or 0
	return function()
		local pos = vim.api.nvim_win_get_cursor(0)
		ev.line = pos[1]
		ev.column = pos[2] + 1
		M.handle(ev)
	end
end

function M.setup(config)
	if not M.defaults.handlers then
		local tbl = {}
		M.defaults.handlers = tbl
		for k, _ in pairs(M.handlers) do
			tbl[#tbl + 1] = k
		end
		M.config = M.defaults
	end
	M.config = config == true and M.defaults or vim.tbl_deep_extend('force', M.config, config)
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], M.genBind(bind[3]))
	end
end

return M
