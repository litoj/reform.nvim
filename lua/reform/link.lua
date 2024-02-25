---@diagnostic disable: need-check-nil
local M = {
	defaults = {
		unknown = 'definition',
		mappings = { { { '', 'i' }, '<C-LeftMouse>' }, { 'n', 'gL' } },
		sorting = function(i) return i end,
	},
}
M.matchers = {
	markdown_url = {
		luapat = '%[.-%]%((https?://[^)]-)%)',
		use = function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	any_url = {
		luapat = '(https?://[%w/#!.:&?=+_%-%%]+)',
		use = function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	markdown_file_uri = {
		luapat = '%[.-%]%(file://([^)]-)%)',
		use = function(file) -- [url_decode](http://lua-users.org/wiki/StringRecipes)
			vim.cmd.e(file:gsub('%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end))
		end,
	},
	markdown_file_path = { luapat = '%[.-%]%(([^)]-)%)', use = vim.cmd.e },
	reform_vimdoc_ref = { luapat = '%[([^%] ]-)%][^(%]]', use = vim.cmd.help },
	vimdoc_ref = { luapat = '|(%S-)|', use = vim.cmd.help },
	stacktrace_file_path = {
		luapat = '(~?[%w._%-]*/[%w/._%-]*:?%d*:?%d*)',
		use = function(path, _, ev)
			local bufname =
				vim.api.nvim_buf_get_name(ev.buf):gsub('term://(.+/)/%d+:.*$', '%1'):gsub('[^/]+$', '')
			local function real(path)
				local pos = path:match ':.+$' or false
				if pos then path = path:sub(1, #path - #pos) end
				for _, f in ipairs { bufname .. path, path:gsub('^~', os.getenv 'HOME') } do
					local ok = io.open(f)
					if ok then
						ok:close()
						return f, pos
					end
				end
				return nil, pos
			end

			local file, pos = real(path)
			if not file then
				local lines = vim.api.nvim_buf_get_lines(ev.buf, ev.line - 1, ev.line + 1, false)
				local src = { lines[1]:find '(~?[%w._%-]*/[%w/._%-]*:?%d*:?%d*)' }
				if not pos and src[2] == #src[3] and lines[2] then -- next line
					file, pos = real(path .. lines[2]:match '^([%w/._%-]+:?%d*:?%d*)')
				end
				if not path:match '^/home' then
					local i = 1
					while not file and ev.line > i and src[3] and src[1] == 1 do
						src = {
							vim.api
								.nvim_buf_get_lines(ev.buf, ev.line - i - 1, ev.line - i, true)[1]
								:find '(~?[%w/._%-]*/[%w/._%-]*)$',
						}
						if src[3] then
							path = src[3] .. path
							file, pos = real(path)
							i = i + 1
						end
					end
				end
			end
			if not file then return false end

			vim.cmd.e(file)
			if pos then
				vim.api.nvim_win_set_cursor(0, {
					tonumber(pos:gsub('^:(%d+).*$', '%1'), 10),
					(tonumber(pos:gsub('^:.+:(%d+)$', '%1'), 10) or 1) - 1,
				})
			end
		end,
	},
	nvim_plugin = {
		luapat = '["\']([%w_%-]+[%w_.%-]*/[%w_.%-]+)[\'"]',
		use = function(url, _, ev)
			if not ({ vim = 1, lua = 1, markdown = 1 })[vim.bo[ev.buf].ft] then return false end
			vim.fn.jobstart(("xdg-open 'https://github.com/%s'"):format(url), { detach = true })
		end,
	},
}
M.defaults.matchers = {
	'markdown_url',
	'any_url',
	'markdown_file_uri',
	'markdown_file_path',
	'reform_vimdoc_ref',
	'vimdoc_ref',
	'stacktrace_file_path',
	'nvim_plugin',
}

function M.handle(ev)
	if not ev.opts then ev.opts = {} end
	if require('reform.util').findMatch(ev, M.config.matchers, M.matchers, M.config.sorting) then
		return
	end

	local cfg = M.config.fallback
	if cfg == 'definition' or ev.opts.mouse then
		if ev.opts.mouse then vim.api.nvim_win_set_cursor(0, { ev.line, ev.column }) end
		return vim.lsp.buf.definition()
	elseif type(cfg) ~= 'table' then
		return vim.notify 'No link found'
	end
	-- create link to current cursor position and copy to clipboard
	local f = io.popen('git config --get remote.origin.url', 'r')
	-- get base url, convert ssh to http url
	local s = {}
	s[#s + 1] = f:read('*l'):gsub('git@(.-):', 'https://%1/'):gsub('%.git$', '')
	-- github uses different path from gitlab
	s[#s + 1] = s[1]:find('github.com', 1, true) and '/blob/' or '/-/blob/'
	f:close()

	local branch
	if cfg.branch == 'current' then -- get current branch name
		f = io.popen('git branch --show-current', 'r')
		branch = f:read '*l' or '' -- head detached → use default branch
		f:close()
		s[#s + 1] = branch
	end
	if cfg.branch == 'default' or branch == '' then
		f = io.popen('git symbolic-ref refs/remotes/origin/HEAD', 'r')
		s[#s + 1] = f:read('*l'):match '[^/]+$' -- trim path to just default branch name
		f:close()
	elseif branch == nil then
		s[#s + 1] = cfg.branch(ev)
	end

	f = io.popen('git rev-parse --show-toplevel', 'r') -- project root
	s[#s + 1] = vim.api.nvim_buf_get_name(0):sub(#f:read '*l' + 1)
	f:close()
	s[#s + 1] = '#L'
	s[#s + 1] = ev.line
	s = table.concat(s)

	if cfg.copy then vim.fn.setreg('+', s) end
	if cfg.print then vim.print(s) end
end

function M.key()
	local pos = vim.api.nvim_win_get_cursor(0)
	M.handle { buf = 0, line = pos[1], column = pos[2] + 1 }
end

function M.mouse()
	local data = vim.fn.getmousepos()
	data.opts = { mouse = true }
	data.buf = vim.api.nvim_win_get_buf(data.winid)
	M.handle(data)
end

M.config = M.defaults
function M.setup(config)
	M.config = config == true and M.defaults or vim.tbl_deep_extend('force', M.config, config)
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], bind[2]:match '[Mm]ouse' and M.mouse or M.key)
	end
end

return M
