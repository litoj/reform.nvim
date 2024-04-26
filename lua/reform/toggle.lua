---@diagnostic disable: undefined-field, assign-type-mismatch
local util = require 'reform.util'

---@type reform.toggle
---@diagnostic disable-next-line: missing-fields
local M = {
	default_config = {
		filter = {
			tolerance = { startPost = math.huge, endPre = 1 },
			sorting = { order = 1, matcher = 3, offset = 1, length = 1 },
		},
	},
}
M.default_config.mapping = {
	inc = { { 'n', 'i' }, '<A-a>', { setCol = 'closer' } },
	dec = { { 'n', 'i' }, '<A-A>', { setCol = 'closer' } },
	{
		{ 'n', 'i' },
		'<A-C-a>',
		{ action = 'tgl', filter = { tolerance = { startPost = 0, endPre = 0 } } },
	},
}
M.config = M.default_config

function M.replace(match, ev, with)
	local line = vim.api.nvim_buf_get_lines(ev.buf, ev.line - 1, ev.line, true)[1]
	line = line:sub(1, match.from - 1) .. with .. line:sub(match.to + 1)
	vim.api.nvim_buf_set_lines(ev.buf, ev.line - 1, ev.line, true, { line })
end
function M.gen_seq_handler(seq, matcher)
	if not matcher then matcher = {} end
	matcher.use = function(val, match, ev)
		local dir = ev.action == 'dec' and -1 or (ev.action == 'inc' and 1 or #seq / 2)
		M.replace(match, ev, seq[(seq[match[1]] - 1 + dir) % #seq + 1])
	end

	if matcher.luapat or matcher.vimre then
		for i, v in ipairs(seq) do -- reverse lookup
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
	answer = M.gen_seq_handler { 'yes', 'no' },
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
	direction = M.gen_seq_handler { 'up', 'north', 'east', 'down', 'south', 'west' },
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
	sign = M.gen_seq_handler { '<', '=', '+', '*', '^', '>', '!', '-', '/', '%' },
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
	toggle = M.gen_seq_handler { 'on', 'off' },
}
M.default_config.matchers =
	{ 'int', 'direction', 'bool', 'logic', 'state', 'toggle', 'answer', 'sign' }

function M.handle(ev)
	local match = util.find_match(ev, M.config.matchers, M.matchers, M.config.filter)
	if not match then return vim.notify 'No toggleable found' end
	if ev.setCol and (match.from > ev.column or match.to < ev.column) then
		local col = ev.setCol == 'end' and match.to or match.from
		if ev.setCol == 'closer' and match.to < ev.column then col = match.to end
		vim.api.nvim_win_set_cursor(0, { ev.line, col - 1 })
	end
	return match
end

--- Generate a function that applies given substitution generator to the current line
---@param matcher {luapat?:string,vimre?:string,(use:fun(val:string,match:reform.util.Match,ev:reform.toggle.Event):string|false),setCursor?:false|fun(match:reform.util.Match,col:integer):integer|false} generates the replacement string
---@return fun() returns generated function that applies the matcher
function M.gen_sub_applicator(matcher)
	local substitutor = matcher.use
	matcher.use = function(val, match, ev)
		local sub = substitutor(val, match, ev)
		if not sub then return false end
		M.replace(match, ev, sub)
		local pos = false --[[@type integer|false]]
		if matcher.setCursor == nil then
			pos = match and match.from <= ev.column and (match.from + #sub - 1)
		elseif matcher.serCursor then
			pos = matcher.setCursor(match, ev.column)
		end
		if pos then vim.api.nvim_win_set_cursor(0, { ev.line, pos }) end
	end
	local ev = { filter = { tolerance = { endPre = 1 } } }
	return function() util.apply_matcher(matcher, ev) end
end

function M.gen_mapping(bind, action)
	local ev = bind[3]
	ev.buf = 0
	ev.action = action or ev.action
	return function()
		local pos = vim.api.nvim_win_get_cursor(0)
		ev.line = pos[1]
		ev.column = pos[2] + 1
		M.handle(ev)
	end
end

return M
