---@diagnostic disable: need-check-nil
---@type reform.link
---@diagnostic disable-next-line: missing-fields
local M = {
	default_config = {
		unknown = 'definition',
		mapping = { mouse = { { '', 'i' }, '<C-LeftMouse>' }, key = { '', 'gL' } },
		filter = { tolerance = { startPost = 1, endPre = 1 } },
	},
}
M.config = M.default_config
M.matchers = {
	markdown_url = {
		luapat = '%[.-%]%((https?://[^)]+)%)',
		use = function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	any_url = {
		luapat = '(https?://[%w/#!.:&?=+_%-%%]+)',
		use = function(url, match, ev)
			if vim.o.columns == match.to then
				url = url .. vim.api.nvim_buf_get_lines(ev.buf, ev.line, ev.line + 1, true)[1]:match '^%S+'
			end
			vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true })
		end,
	},
	markdown_file_uri = {
		luapat = '%[.-%]%(file://([^)]+)%)',
		use = function(file) -- [url_decode](http://lua-users.org/wiki/StringRecipes)
			vim.cmd.e(file:gsub('%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end))
		end,
	},
	markdown_file_path = {
		luapat = '%[.-%]%(([^)]+)%)',
		use = function(match)
			local f = io.open(match)
			if not f then return end
			f:close()
			vim.cmd.e(match)
		end,
	},
	reform_vimdoc_ref = {
		luapat = '%[([^%] ]-)%][^(%]]',
		use = function(match) return pcall(vim.cmd.help, match) end,
	},
	vimdoc_ref = { luapat = '|(%S-)|', use = function(match) return pcall(vim.cmd.help, match) end },
	stacktrace_file_path = {
		luapat = '(~?[%w/.@_%-]+:?%d*:?%d*)',
		use = function(path, matches, ev)
			local bufdir = vim.api
				.nvim_buf_get_name(ev.buf)
				:gsub('^term://(.+/)/%d+:.*$', '%1', 1)
				:gsub('^~', os.getenv 'HOME', 1)
			local cwd = vim.loop.cwd()
			local function exists(f)
				f = io.open(f)
				if f then f:close() end
				return f ~= nil
			end
			local function real(path)
				local pos = path:match ':.+$' or false
				if pos then path = path:sub(1, #path - #pos) end
				path = path:gsub('^~', os.getenv 'HOME', 1)

				if exists(path) then return path, pos end
				local dir = bufdir .. path
				if exists(dir) then return dir, pos end
				dir = cwd:gsub('[^/]+$', path, 1) -- src/ is often in both cwd and path
				return exists(dir) and dir or nil, pos
			end

			local file, pos = real(path)
			if not file then
				local lines = vim.api.nvim_buf_get_lines(ev.buf, ev.line - 1, ev.line + 1, false)
				local src = { matches.from, matches.to, matches[1] }
				if not pos and src[2] == #lines[1] and lines[2] then -- next line
					pos = lines[2]:match '^%s*([%w/._%-]*:?%d+:?%d*)' -- TODO: make this run for OK file
					path = path .. (pos or '')
					file, pos = real(path)
				end
				if not path:match '^/home' then
					local i = 1
					while not file and ev.line > i and src[3] and src[1] == 1 do
						src = {
							vim.api
								.nvim_buf_get_lines(ev.buf, ev.line - i - 1, ev.line - i, true)[1]
								:find '(~?[%w/._%-]+)$',
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
		luapat = '["\'](%w+[%w_.%-]*/[%w_.%-]+)[\'"]',
		use = function(url, _, ev)
			if not ({ vim = 1, lua = 1, markdown = 1 })[vim.bo[ev.buf].ft] then return false end
			vim.fn.jobstart(("xdg-open 'https://github.com/%s'"):format(url), { detach = true })
		end,
	},
}
M.default_config.matchers = {
	'markdown_url',
	'any_url',
	'markdown_file_uri',
	'markdown_file_path',
	'reform_vimdoc_ref',
	'vimdoc_ref',
	'stacktrace_file_path',
	'nvim_plugin',
}

function M.get_git_url(use_default_branch, from, to)
	-- create link to current cursor position and copy to clipboard
	local f = io.popen('git config --get remote.origin.url', 'r')
	-- get base url, convert ssh to http url
	local s = {}
	s[#s + 1] = f:read '*l'
	if #s == 0 then return end
	s[1] = s[1]:gsub('git@(.-):', 'https://%1/'):gsub('%.git$', '')
	-- github uses different path from gitlab
	local isGithub = s[1]:find('github.com', 1, true)
	s[#s + 1] = isGithub and '/blob/' or '/-/blob/'
	f:close()

	local branch = ''
	if not use_default_branch then -- get current branch name
		f = io.popen('git branch', 'r')
		while not vim.startswith(branch, '* ') do
			branch = f:read '*l'
		end

		if vim.startswith(branch, '* (HEAD detached at') then
			branch = branch:match ' (%S+)%)$'
			if vim.startswith(branch, 'origin/') then branch = branch:sub(8) end
		else
			branch = branch:match ' (%S+)$'
		end

		f:close()
	else
		f = io.popen('git symbolic-ref refs/remotes/origin/HEAD', 'r')
		branch = f:read('*l'):match '[^/]+$' -- trim path to just default branch name
		f:close()
	end
	s[#s + 1] = branch

	f = io.popen('git rev-parse --show-toplevel', 'r') -- project root
	s[#s + 1] = vim.api.nvim_buf_get_name(0):sub(#f:read '*l' + 1)
	f:close()

	s[#s + 1] = '#L'
	s[#s + 1] = from
	if to then
		s[#s + 1] = isGithub and '-L' or '-'
		s[#s + 1] = to
	end

	return table.concat(s)
end

function M.handle(ev)
	local from, to = vim.fn.getpos('.')[2], vim.fn.getpos('v')[2]
	if from > to then
		local tmp = to
		to = from
		from = tmp
	end

	local cfg = M.config.fallback

	if from == to or ev.mouse then -- not visual mode - ^$ on the same line or mouse click
		from = ev.line
		to = nil

		if require('reform.util').find_match(ev, M.config.matchers, M.matchers, M.config.filter) then
			return
		end

		if cfg == 'definition' or ev.mouse then
			if ev.mouse then vim.api.nvim_win_set_cursor(0, { ev.line, ev.column }) end
			return vim.lsp.buf.definition()
		elseif type(cfg) ~= 'table' then
			return vim.notify 'No link found'
		end
	end -- only get git url in visual mode

	local link = M.get_git_url(cfg.branch == 'default', from, to)
	if not link then return vim.notify 'No git repo found' end

	if cfg.copy then vim.fn.setreg('+', link) end
	if cfg.print then vim.print(link) end
end

function M.key()
	local pos = vim.api.nvim_win_get_cursor(0)
	return M.handle { buf = 0, line = pos[1], column = pos[2] + 1 }
end

function M.mouse()
	local data = vim.fn.getmousepos()
	data.mouse = true
	data.buf = vim.api.nvim_win_get_buf(data.winid)
	return M.handle(data)
end

function M.gen_mapping(bind) return bind[2]:match '[Mm]ouse' and M.mouse or M.key end

return M
