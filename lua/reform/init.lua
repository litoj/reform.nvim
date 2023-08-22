local M = { config = { input = true, select = true, docmd = true, open_link = true } }

return {
	---@param config? reform.Config
	setup = function(config)
		if config then M.config = vim.tbl_deep_extend('force', M.config, config) end
		for k, v in pairs(M.config) do
			if v or package.loaded['reform.' .. k] then require('reform.' .. k)(v) end
		end
	end,
}
