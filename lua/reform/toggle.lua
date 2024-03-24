local M = {
	defaults = {
		filter = {
			tolerance = { startPost = math.huge, endPre = 1 },
			sorting = { order = 1, matcher = 3, offset = 1, length = 1 },
		},
	},
}
M.defaults.mappings = {
	{ { 'n', 'i' }, '<A-a>', { action = 'inc', setCol = 'closer' } },
	{ { 'n', 'i' }, '<A-A>', { action = 'dec', setCol = 'closer' } },
	{
		{ 'n', 'i' },
		'<A-C-a>',
		{ action = 'tgl', filter = { tolerance = { startPost = 0, endPre = 0 } } },
	},
}

function M.replace(match, ev, with)
	local line = vim.api.nvim_buf_get_lines(ev.buf, ev.line - 1, ev.line, true)[1]
	line = line:sub(1, match.from - 1) .. with .. line:sub(match.to + 1)
	vim.api.nvim_buf_set_lines(ev.buf, ev.line - 1, ev.line, true, { line })
end
function M.genSeqHandler(seq, matcher)
	if not matcher then matcher = {} end
	matcher.use = function(val, match, ev)
		local dir = ev.action == 'dec' and -1 or (ev.action == 'inc' and 1 or #seq / 2)
		M.replace(match, ev, seq[(seq[match[1]] - 1 + dir) % #seq + 1])
	end

	if matcher.luapat or matcher.vimre then
		for i, v in ipairs(seq) do
			seq[v] = i
		end
	else
		local pat = {}
		local escape = '^]%-'
		if #seq[1] == 1 then
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = escape:find(v, 0, true) and '%' .. v or v
			end
			matcher.luapat = '([' .. table.concat(pat, '') .. '])'
		else
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = v:gsub('[\\.*[%]]', '\\%0')
			end
			matcher.vimre = [[\<\(]] .. table.concat(pat, '\\|') .. [[\)\>]]
		end
	end

	return matcher
end

M.matchers = {
	answer = M.genSeqHandler { 'yes', 'no' },
	bool = {
		vimre = [[\<\([Tt]rue\|[Ff]alse\)\>]],
		group = 3,
		use = function(val, match, ev)
			if match[1]:byte(2) == 114 then
				M.replace(match, ev, val:byte() == 84 and 'False' or 'false')
			else
				M.replace(match, ev, val:byte() == 70 and 'True' or 'true')
			end
		end,
	},
	direction = M.genSeqHandler { 'up', 'north', 'east', 'down', 'south', 'west' },
	int = {
		vimre = [[-\?\d\+]],
		group = 1,
		use = function(val, match, ev)
			local num = tonumber(val)
			if ev.action == 'tgl' then
				M.replace(match, ev, tostring(-num))
			else
				M.replace(match, ev, tostring(num + (ev.action == 'inc' and 1 or -1)))
			end
		end,
	},
	logic = {
		vimre = [[[&|]\+\|\<and\>\|\<or\>]],
		use = function(val, match, ev)
			if val:sub(1, 1) == '&' or val:sub(1, 1) == '|' then
				if vim.bo[ev.buf].ft == 'lua' then return false end
				M.replace(match, ev, (val:sub(1, 1) == '|' and { '&', '&&' } or { '|', '||' })[#val])
			else
				if vim.bo[ev.buf].ft ~= 'lua' then return false end
				M.replace(match, ev, val == 'or' and 'and' or 'or')
			end
		end,
	},
	sign = M.genSeqHandler { '<', '=', '+', '*', '^', '>', '!', '-', '/', '%' },
	state = {
		vimre = [[\<\(enabled\?\|disabled\?\)\>]],
		use = function(_, match, ev)
			if match[1]:byte() == 101 then
				M.replace(match, ev, match[1]:byte(-1) == 100 and 'disabled' or 'disable')
			else
				M.replace(match, ev, match[1]:byte(-1) == 100 and 'enabled' or 'enable')
			end
		end,
	},
	toggle = M.genSeqHandler { 'on', 'off' },
}
M.defaults.matchers = { 'int', 'direction', 'bool', 'logic', 'state', 'toggle', 'answer', 'sign' }

function M.handle(ev)
	local match = require('reform.util').findMatch(ev, M.config.matchers, M.matchers, M.config.filter)
	if not match then return vim.notify 'No toggleable found' end
	if ev.setCol and (match.from > ev.column or match.to < ev.column) then
		local col = ev.setCol == 'end' and match.to or match.from
		if ev.setCol == 'closer' and match.to < ev.column then col = match.to end
		vim.api.nvim_win_set_cursor(0, { ev.line, col - 1 })
	end
	return match
end

--- Generate a function that applies given substitution generator to the current line
---@param matcher {luapat?:string,vimre?:string,use:fun(match:string,info:reform.util.Match,ev:reform.util.Event):string|false} generates the replacement string
---@return fun() returns generated function that applies the matcher
function M.genSubApplicator(matcher)
	local substitutor = matcher.use
	local sub
	matcher.use = function(val, match, ev)
		sub = substitutor(val, match, ev)
		if sub then
			M.replace(match, ev, sub)
		else
			return false
		end
	end
	local ev = { buf = 0, filter = { tolerance = { endPre = 1 } } }
	return function()
		local pos = vim.api.nvim_win_get_cursor(0)
		ev.line = pos[1]
		ev.column = pos[2] + 1
		local match = require('reform.util').findMatch(ev, { matcher }, {}, {})
		if match and match.from < ev.column then
			vim.api.nvim_win_set_cursor(0, { ev.line, match.from + #sub - 1 })
		end
	end
end

function M.genBind(ev)
	ev.buf = 0
	return function()
		local pos = vim.api.nvim_win_get_cursor(0)
		ev.line = pos[1]
		ev.column = pos[2] + 1
		M.handle(ev)
	end
end

M.config = M.defaults
function M.setup(config)
	M.config = config == true and M.defaults or vim.tbl_deep_extend('force', M.config, config)
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], M.genBind(bind[3]))
	end
end

return M
