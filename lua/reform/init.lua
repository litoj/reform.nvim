local M = {config = {select = true, docmd = true, open_link = true}, state = {}}

return {
	---Set configuration and initiate reform.nvim
	---@param config? {select: boolean|function, docmd: boolean|table, open_link: boolean|table}
	---@see https://github.com/JosefLitos/reform.nvim
	setup = function(config)
		if config then M.config = vim.tbl_deep_extend("force", M.config, config) end
		for k, v in pairs(M.config) do
			if v or package.loaded["reform." .. k] then M.state[k] = require("reform." .. k)(v) end
		end
	end,
}
