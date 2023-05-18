local M = {config = {input = true, select = true, docmd = true, open_link = true}}

return {
	---Set configuration and initiate reform.nvim
	---@param config? {input?: boolean|function|table, select?: boolean|function|table, docmd?: boolean|table, open_link?: boolean|table}
	---@see https://github.com/JosefLitos/reform.nvim
	setup = function(config)
		if config then M.config = vim.tbl_deep_extend("force", M.config, config) end
		for k, v in pairs(M.config) do
			if v or package.loaded["reform." .. k] then require("reform." .. k)(v) end
		end
	end,
}
