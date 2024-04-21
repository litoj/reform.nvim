---@diagnostic disable: param-type-mismatch, redefined-local
---@type reform
---@diagnostic disable-next-line: missing-fields
local M = {
	default_config = {
		ui = true,
		docmd = true,
		link = true,
		toggle = true,
		tbl_extras = false,
		sig_help = true,
	},
}
M.config = M.default_config

function M.set_override(override, cfg)
	for k, v in pairs(cfg or {}) do
		if v then
			override.set[k](type(v) == 'function' and v or override.reform[k])
		else
			override.set[k](override.vim[k])
		end
	end
end

function M.set_mapping(functions, cfg)
	for k, bind in pairs(cfg and (type(cfg[2]) == 'string' and { cfg } or cfg) or {}) do
		if type(bind[2]) == 'string' then bind = { bind } end
		local action = functions[k]
		for _, bind in ipairs(bind) do
			vim.keymap.set(
				bind[1],
				bind[2],
				action and (bind[3] and function() action(bind[3]) end or action)
					or functions.gen_mapping(bind, type(k) == 'string' and k)
			)
		end
	end
end

function M.setup(config)
	if config then M.config = vim.tbl_deep_extend('force', M.config, config) end
	local set_override = M.set_override
	local set_mapping = M.set_mapping

	for k, config in pairs(M.config) do
		local module = 'reform.' .. k
		if config or package.loaded[module] then
			local M = require(module)

			if type(config) == 'boolean' then
				if config then
					M.config = M.default_config
				else
					M.config = {}
					for k, _ in pairs(M.default_config.override or {}) do
						M.config[k] = false
					end
					M.config = { override = M.config } -- no new mapping, but neither unmapping
				end
			else
				M.config = vim.tbl_deep_extend('force', M.config, config)
			end

			set_override(M.override, M.config.override)

			set_mapping(M, M.config.mapping)

			if M.setup then M.setup(config) end
		end
	end
end

return M
