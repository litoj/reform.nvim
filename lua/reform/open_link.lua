---@diagnostic disable: need-check-nil
-- originally made by [nvim-lsp-extras](https://github.com/seblj/nvim-lsp-extras/blob/1cc94a8e590da48284e103e9a4627f7fbb178618/lua/nvim-lsp-extras/treesitter_hover/markdown.lua#L130)
local M = {
	defaults = {
		unknown = 'definition',
		mappings = { { { '', 'i' }, '<C-LeftMouse>' }, { 'n', 'gL' } },
	},
}
M.handlers = { -- TODO: be filetype specific/dynamically choose by external function
	markdown_url = {
		pattern = '%[.-%]%((https?://[^)]-)%)',
		use = function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	any_url = {
		pattern = '(https?://[%w/#!.:&?=+_%-%%]+)',
		use = function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	markdown_file_uri = {
		pattern = '%[.-%]%(file://([^)]-)%)',
		use = function(file) -- [url_decode](http://lua-users.org/wiki/StringRecipes)
			vim.cmd.e(file:gsub('%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end))
		end,
	},
	markdown_file_path = { pattern = '%[.-%]%(([^)]-)%)', use = vim.cmd.e },
	reform_vimdoc_ref = { pattern = '%[([^%] ]-)%][^(%]]', use = vim.cmd.help },
	vimdoc_ref = { pattern = '|(%S-)|', use = vim.cmd.help },
	any_file_path = {
		pattern = '(~?[%w._%-]*/[%w/._%-]*:?%d*:?%d*)',
		use = function(file) -- TODO: try join lines to find complete path (prev and post)
			local pos = file:match ':.+$' or false
			if pos then file = file:match '^[^:]+' end
			local open
			for _, f in ipairs {
				vim.api.nvim_buf_get_name(0):gsub('term://(.+/)/%d+:.*$', '%1'):gsub('[^/]+$', '') .. file,
				file:gsub('^~', os.getenv 'HOME'),
			} do
				local ok = io.open(f)
				if ok then
					open = f
					ok:close()
					break
				end
			end
			if not open then return 1 end
			vim.cmd.e(open)

			if pos then
				vim.api.nvim_win_set_cursor(0, {
					tonumber(pos:gsub('^:(%d+).*$', '%1'), 10),
					tonumber(pos:gsub('^:.+:(%d+)$', '%1'), 10) or 0,
				})
			end
		end,
	},
	nvim_plugin = {
		pattern = '["\']([%w_-]+[%w_.%-]+/[%w_.%-]+)[\'"]',
		use = function(url, ev)
			if not ({ vim = 1, lua = 1, markdown = 1 })[vim.bo[ev.buf].ft] then return 1 end
			vim.fn.jobstart(("xdg-open 'https://github.com/%s'"):format(url), { detach = true })
		end,
	},
}
M.defaults.handlers = {
	'markdown_url',
	'any_url',
	'markdown_file_uri',
	'markdown_file_path',
	'reform_vimdoc_ref',
	'vimdoc_ref',
	'any_file_path',
	'nvim_plugin',
}

function M.open_link(ev)
	local line = vim.api.nvim_buf_get_lines(ev.buf, ev.line - 1, ev.line, true)[1]
	local col = ev.column
	local cfg = M.config

	for _, handler in ipairs(type(cfg.handlers) == 'function' and cfg.handlers(ev) or cfg.handlers) do
		if type(handler) == 'string' then handler = M.handlers[handler] end
		local from, to, url = line:find(handler.pattern, 1)
		while from do
			if col >= from and col <= to and not handler.use(url, ev) then return end -- parsing success
			from, to, url = line:find(handler.pattern, to + 1)
		end
	end

	cfg = cfg.fallback
	if cfg == 'definition' or ev.mouse then
		if ev.mouse then vim.api.nvim_win_set_cursor(0, { ev.line, ev.column }) end
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
	M.open_link { buf = 0, line = pos[1], column = pos[2] + 1 }
end

function M.mouse()
	local data = vim.fn.getmousepos()
	data.mouse = true
	data.buf = vim.api.nvim_win_get_buf(data.winid)
	M.open_link(data)
end

M.config = M.defaults
function M.setup(config)
	M.config = config == true and M.defaults or vim.tbl_deep_extend('force', M.config, config)
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], bind[2]:match '[Mm]ouse' and M.mouse or M.key)
	end
end

return M
