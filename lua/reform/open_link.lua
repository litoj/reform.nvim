-- originally made by [nvim-lsp-extras](https://github.com/seblj/nvim-lsp-extras/blob/1cc94a8e590da48284e103e9a4627f7fbb178618/lua/nvim-lsp-extras/treesitter_hover/markdown.lua#L130)
local M = {
	defaults = {
		unknown = 'definition',
		mappings = { { { '', 'i' }, '<C-LeftMouse>' }, { 'n', 'gL' } },
	},
}
M.defaults.handlers = { -- TODO: be filetype specific/dynamically choose by external function
	{
		'%[.-%]%((https?://[^)]-)%)',
		function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	{
		'(https?://[%w/#!.:&?=+_%-%%]+)',
		function(url) vim.fn.jobstart(("xdg-open '%s'"):format(url), { detach = true }) end,
	},
	{
		'%[.-%]%(file://([^)]-)%)',
		function(file) -- [url_decode](http://lua-users.org/wiki/StringRecipes)
			vim.cmd.e(file:gsub('%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end))
		end,
	},
	{ '%[.-%]%(([^)]-)%)', vim.cmd.e },
	{ '%[([^%] ]-)%][^(%]]', vim.cmd.help },
	{ '|(%S-)|', vim.cmd.help },
	{
		'(~?[%w._%-]*/[%w/._%-]*:?%d*:?%d*)',
		function(file)
			local pos = file:match ':.+$' or false
			if pos then file = file:match '^[^:]+' end
			vim.cmd.e(
				file:match '^[~/]' and file
					or (
						vim.api.nvim_buf_get_name(0):gsub('term://(.+/)/%d+:.*$', '%1'):gsub('[^/]+$', '')
						.. file
					)
			)
			if pos then
				vim.api.nvim_win_set_cursor(0, {
					tonumber(pos:gsub('^:(%d+).*$', '%1'), 10),
					tonumber(pos:gsub('^:.+:(%d+)$', '%1'), 10) or 0,
				})
			end
		end,
	},
	{
		'.+',
		function(word, ev)
			if M.config.unknown == 'definition' or ev.mouse then
				if ev.mouse then vim.api.nvim_win_set_cursor(0, { ev.line, ev.column }) end
				return vim.lsp.buf.definition()
			end
			-- create link to current cursor position and copy to clipboard
			local f = io.popen('git config --get remote.origin.url', 'r')
			-- get base url, convert ssh to http url
			local s = {}
			s[#s + 1] = f:read('*l'):gsub('git@(.-):', 'https://%1/'):gsub('%.git$', '')
			-- github uses different path from gitlab
			s[#s + 1] = s[1]:match 'github.com' and '/blob/' or '/-/blob/'
			f:close()

			if M.config.unknown:match 'default' then
				f = io.popen('git symbolic-ref refs/remotes/origin/HEAD', 'r')
				s[#s + 1] = f:read('*l'):match '[^/]+$' -- trim path to just default branch name
				f:close()
			elseif M.config.unknown:match 'current' then
				f = io.popen('git branch --show-current', 'r') -- get current branch name
				s[#s + 1] = f:read '*l'
				f:close()
			else
				return vim.notify 'No link found'
			end

			f = io.popen('git rev-parse --show-toplevel', 'r') -- project root
			s[#s + 1] = vim.api.nvim_buf_get_name(0):sub(#f:read '*l' + 1)
			f:close()
			s[#s + 1] = '#L'
			s[#s + 1] = ev.line - 1
			s = table.concat(s)

			if M.config.unknown:match 'copy' then
				vim.fn.setreg('+', s)
			elseif M.config.unknown:match 'print' then
				vim.print(s)
			end
		end,
	},
}

function M.open_link(buf, ev)
	local line = vim.api.nvim_buf_get_lines(buf, ev.line - 1, ev.line, true)[1]
	local col = ev.column

	for _, pairs in ipairs(M.config.handlers) do
		local from, to, url = line:find(pairs[1], 1)
		while from do
			if col >= from and col <= to then return pairs[2](url, ev) end -- found
			from, to, url = line:find(pairs[1], to + 1)
		end
	end
end
---@type reform.open_link
M = setmetatable(M, { __call = M.open_link })

function M.key()
	local pos = vim.api.nvim_win_get_cursor(0)
	M.open_link(0, { line = pos[1], column = pos[2] + 1 })
end

function M.mouse()
	local data = vim.fn.getmousepos()
	data.mouse = true
	M.open_link(vim.api.nvim_win_get_buf(data.winid), data)
end

M.config = M.defaults
function M.setup(config)
	M.config = config == true and M.defaults or vim.tbl_deep_extend('force', M.config, config)
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], bind[2]:match 'Mouse' and M.mouse or M.key)
	end
end

return M
