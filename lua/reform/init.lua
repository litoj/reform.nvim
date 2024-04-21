---@type reform
---@diagnostic disable-next-line: missing-fields
local M = {
	defaults = {
		input = true,
		select = true,
		docmd = true,
		link = true,
		toggle = true,
		tbl_extras = false,
		sig_help = true,
	},
}
M.config = M.defaults

function M.setup(config)
	if config then M.config = vim.tbl_deep_extend('force', M.config, config) end
	for k, v in pairs(M.config) do
		local module = 'reform.' .. k
		if v or package.loaded[module] then require(module).setup(v) end
	end
end

return M
