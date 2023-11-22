-- originally made by [nvim-lsp-extras](https://github.com/seblj/nvim-lsp-extras/blob/1cc94a8e590da48284e103e9a4627f7fbb178618/lua/nvim-lsp-extras/treesitter_hover/markdown.lua#L130)
local M = {
	defaults = {
		mappings = { { { '', 'i' }, '<C-LeftMouse>' }, { 'n', 'gl' } },
		handlers = {
			{
				'%[.-%]%((https?://[^)]-)%)',
				function(url) vim.fn.jobstart('xdg-open ' .. url, { detach = true }) end,
			},
			{
				'%[.-%]%(file://([^)]-)%)',
				function(file) -- [url_decode](http://lua-users.org/wiki/StringRecipes)
					vim.cmd.e(file:gsub('%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end))
				end,
			},
			{ '%[.-%]%(([^)]-)%)', vim.cmd.e },
			{ '%[([^%] ]-)%][^(%]]', vim.cmd.help },
			{ '|([^% ]]-)|', vim.cmd.help },
			{
				'(https?://[/#!.:&?=+0-9A-Za-z_-%%]+)',
				function(url) vim.fn.jobstart("xdg-open '" .. url .. "'", { detach = true }) end,
			},
			{ '(~?[0-9A-Za-z._-]*/[/0-9A-Za-z._-]*)', vim.cmd.e },
		},
	},
}
M.config = M.defaults

function M.open_link(line, col)
	for _, data in ipairs(M.config.handlers) do
		local from = 1
		local to, url
		while from do
			from, to, url = line:find(data[1], from)
			if from and col >= from and col <= to then return data[2](url) end
			if from then from = to + 1 end
		end
	end
	vim.print 'No link found'
end
---@type reform.open_link
M = setmetatable(M, { __call = M.open_link })

function M.key() M.open_link(vim.api.nvim_get_current_line(), vim.api.nvim_win_get_cursor(0)[2] + 1) end

function M.mouse()
	local data = vim.fn.getmousepos()
	M.open_link(
		vim.api.nvim_buf_get_lines(vim.api.nvim_win_get_buf(data.winid), data.line - 1, data.line, true)[1],
		data.column
	)
end

function M.setup(config)
	if config == true then config = M.defaults end
	M.config = vim.tbl_deep_extend('force', M.config, config)
	if config.mappings then
		for _, bind in ipairs(config.mappings) do
			vim.keymap.set(bind[1], bind[2], bind[2]:match 'Mouse' and M.mouse or M.key)
		end
	end
end

return M
