---@type reform
local M =
	---@diagnostic disable-next-line: missing-fields
	{ defaults = { input = true, select = true, docmd = true, open_link = true, man = false } }
M.config = M.defaults

function M.setup(config)
	if config then M.config = vim.tbl_deep_extend('force', M.config, config) end
	for k, v in pairs(M.config) do
		local module = 'reform.' .. k
		if v or package.loaded[module] then
			M[k] = require(module)
			M[k].setup(v)
		end
	end
end

return M
