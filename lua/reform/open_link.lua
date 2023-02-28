-- originally made by [nvim-lsp-extras](https://github.com/seblj/nvim-lsp-extras/blob/master/lua/nvim-lsp-extras/treesitter_hover/markdown.lua#L148)
local M = {default = {{{"", "i"}, "<C-LeftMouse>"}, {"n", "gl"}}}
function M.open_link()
	local line = vim.api.nvim_get_current_line()
	local pos = vim.api.nvim_win_get_cursor(0)
	local col = pos[2] + 1

	local hover = { -- using array to keep order of testing
		{"%[.-%]%((http[^)]-)%)", function(url) vim.fn.jobstart("xdg-open " .. url, {detach = true}) end},
		{
			"%[.-%]%(file://([^)]-)%)",
			function(file)
				-- relying on [urlencode](https://github.com/AquilaIrreale/urlencode) for now
				file = vim.fn.system("urlencode -d", file)
				vim.cmd.e(file)
			end,
		},
		{"%[.-%]%(([^)]-)%)", vim.cmd.e},
		{"%[([^%]]-)%][^(]", vim.cmd.help},
	}

	for _, data in ipairs(hover) do
		local from = 1
		local to, url
		while from do
			from, to, url = line:find(data[1], from)
			if from and col >= from and col <= to then return data[2](url) end
			if from then from = to + 1 end
		end
	end
	vim.notify("No link found", vim.log.levels.INFO)
end

return function(config)
	if config then config = M.default end
	if config then for _, bind in ipairs(config) do vim.keymap.set(bind[1], bind[2], M.open_link) end end
	return M
end
