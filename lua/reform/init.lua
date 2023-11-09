local M = { config = { input = true, select = true, docmd = true, open_link = true, man = true } }

---@param config? reform.Config
function M.setup(config)
	if config then M.config = vim.tbl_deep_extend('force', M.config, config) end
	for k, v in pairs(M.config) do
		if v or package.loaded['reform.' .. k] then require('reform.' .. k).setup(v) end
	end
end

return M
