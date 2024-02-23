local M = {
	defaults = {
		sorting = { order = 1, matcher = -3, offset = 1, length = 1 },
	},
}
function M.setColToStart(event, match) return event.column < match.from and match.from - 1 end
M.defaults.mappings = {
	{ { 'n', 'i' }, '<A-a>', { action = 'inc', noFromCheck = true, setCol = M.setColToStart } },
	{ { 'n', 'i' }, '<A-A>', { action = 'dec', noFromCheck = true, setCol = M.setColToStart } },
	{ { 'n', 'i' }, '<A-C-a>', { action = 'tgl' } },
}

function M.replace(match, event, with)
	local line = vim.api.nvim_buf_get_lines(event.buf, event.line - 1, event.line, true)[1]
	line = line:sub(1, match.from - 1) .. with .. line:sub(match.to + 1)
	vim.api.nvim_buf_set_lines(event.buf, event.line - 1, event.line, true, { line })
end
function M.genSeqHandler(seq, matcher)
	if not matcher then matcher = {} end
	matcher.use = function(_, match, ev)
		local dir = ev.opts.action == 'dec' and -1 or (ev.opts.action == 'inc' and 1 or #seq / 2)
		M.replace(match, ev, seq[(seq[match[1]] - 1 + dir) % #seq + 1])
	end
	if matcher.luapat or matcher.vimre then
		for i, v in ipairs(seq) do
			seq[v] = i
		end
	else
		local pat = {}
		if #seq[1] == 1 then
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = (v == ']' or v == '%') and '%' .. v or v
			end
			matcher.luapat = '([' .. table.concat(pat, '') .. '])'
		else
			for i, v in ipairs(seq) do
				seq[v] = i
				pat[i] = v:gsub('[\\.*[%]]', '\\%0')
			end
			matcher.vimre = table.concat(pat, '\\|')
		end
	end
	return matcher
end

M.matchers = {
	answer = M.genSeqHandler({ 'yes', 'no' }, { vimre = [[\<\(yes\|no\)\>]] }),
	bool = {
		vimre = [[[Tt]rue\|[Ff]alse]],
		weight = 5,
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
		weight = 10,
		use = function(_, match, event)
			local num = tonumber(match[1])
			if event.opts.action == 'tgl' then
				M.replace(match, event, tostring(-num))
			else
				M.replace(match, event, tostring(num + (event.opts.action == 'inc' and 1 or -1)))
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
M.defaults.matchers =
	{ 'int', 'direction', 'bool', 'comparator', 'logic', 'state', 'toggle', 'answer', 'sign' }

function M.handle(ev)
	if not ev.opts then ev.opts = {} end
	if require('reform.util').findMatch(ev, M.config.matchers, M.matchers, M.config.sorting) then
		return
	end
	vim.notify 'No toggleable found'
end

function M.genBind(evOpts)
	local ev = { buf = 0, opts = evOpts or {} }
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
