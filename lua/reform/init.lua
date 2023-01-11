local M = {config = {input = true, select = true, docmd = true}}

return {
	---Set configuration and initiate reform.nvim
	---@param config? {input: boolean, select: boolean, docmd: boolean}
	---@see https://github.com/JosefLitos/reform.nvim
	setup = function(config)
		M.config = vim.tbl_deep_extend("force", M.config, config or {})
		for k, v in pairs(M.config) do
			if v or package.loaded["reform." .. k] then require("reform." .. k)(v) end
		end
	end,
}
