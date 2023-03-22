-- originally made by [nvim-lsp-extras](https://github.com/seblj/nvim-lsp-extras/blob/master/lua/nvim-lsp-extras/treesitter_hover/markdown.lua#L148)
local M = {default = {{{"", "i"}, "<C-LeftMouse>"}, {"n", "gl"}}, handlers = {}}
function M.handlers.open_link(line, col)
	local hover = { -- using array to keep order of testing
		{"%[.-%]%((https?://[^)]-)%)", function(url) vim.fn.jobstart("xdg-open " .. url, {detach = true}) end},
		{
			"%[.-%]%(file://([^)]-)%)",
			function(file)
				-- relying on [urlencode](https://github.com/AquilaIrreale/urlencode) for now
				file = vim.fn.system("urlencode -d", file)
				vim.cmd.e(file)
			end,
		},
		{"%[.-%]%(([^)]-)%)", vim.cmd.e},
		{"%[([^%] ]-)%][^(%]]", vim.cmd.help},
		{"|([^% ]]-)|", vim.cmd.help},
		{"(https?://[^ \t()[%]{}]+)", function(url) vim.fn.jobstart("xdg-open " .. url, {detach = true}) end},
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

function M.handlers.key()
	M.handlers.open_link(vim.api.nvim_get_current_line(), vim.api.nvim_win_get_cursor(0)[2] + 1)
end

function M.handlers.mouse()
	local data = vim.fn.getmousepos()
	M.handlers.open_link(vim.api.nvim_buf_get_lines(vim.api.nvim_win_get_buf(data.winid),
			                     data.line - 1, data.line, true)[1], data.column)
end

return function(config)
	if config then config = M.default end
	if config then
		for _, bind in ipairs(config) do
			vim.keymap.set(bind[1], bind[2], M.handlers[bind[2]:match("Mouse") and "mouse" or "key"])
		end
	end
	return M
end
