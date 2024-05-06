---@type reform.tbl_extras
---@diagnostic disable-next-line: missing-fields
local M = {
	override = {
		vim = { print = _G.print },
		set = { print = function(fn) _G.print = fn end },
		reform = {},
	},
	default_config = {
		diff = { expand_unique = '…' },
		cut_depth = { depth = 2, cuts = {} },
		override = { print = true },
	},
}
M.config = M.default_config

local function tbl_complement(opts, src, to_complement)
	local diff = opts.diff_table or {}
	local suff = opts.attr_suffix

	for k, v1 in pairs(src) do
		if diff[k .. suff] == nil then
			local v2 = to_complement[k]
			if type(v1) == 'table' and type(v2) == 'table' then
				opts.diff_table = diff[k]
				diff[k] = tbl_complement(opts, v1, v2)
			elseif v1 ~= v2 then
				diff[k .. suff] = (type(v1) == 'table' and opts.expand_unique ~= true)
						and opts.expand_unique
					or v1
			end
		end
	end
	return diff
end

function M.tbl_diff(opts, src, ...)
	local all = { ... }
	if #all == 0 then
		all = { src }
		src = opts
		opts = {}
	end
	opts = vim.tbl_extend('force', M.config.diff, { diff_table = {} }, opts or {})

	opts.attr_suffix = ' 0'
	opts.diff_table = tbl_complement(opts, src, vim.tbl_deep_extend('force', {}, unpack(all)))

	for i, cmp in ipairs(all) do
		opts.attr_suffix = ' ' .. i
		opts.diff_table = tbl_complement(opts, cmp, src)
	end
	return next(opts.diff_table) ~= nil and opts.diff_table or nil
end
function M.tbl_short_diff(...) return M.tbl_diff({ expand_unique = '…' }, ...) end
function M.tbl_full_diff(...) return M.tbl_diff({ expand_unique = true }, ...) end

function M.tbl_cut_depth(tbl, opts)
	opts = vim.tbl_extend('force', M.config.cut_depth, opts or {})
	local depth = opts.depth

	local copy = {}
	for k, v in pairs(tbl) do
		if type(v) == 'table' then
			if depth > 0 then
				opts.depth = depth - 1
				copy[k] = M.tbl_cut_depth(v, opts)
			elseif opts.cuts then
				copy[k] = opts.cuts
			end
		else
			copy[k] = v
		end
	end
	return copy
end

function M.tbl_print(tbl)
	tbl = vim.inspect(tbl)
	vim.schedule(function() vim.notify(tbl) end)
end

--- global printer extension for tables - 1 tbl=print with default depth, 2+ tbls=print with diff to first
function M.override.reform.print(...)
	local bundled = { ... }
	local x, y = bundled[1], bundled[2]
	if type(x) ~= 'table' then
		M.override.vim.print(...)
	elseif type(y) == 'table' then
		M.tbl_print(M.tbl_diff({}, x, y, ...) or {})
	elseif y == nil or type(y) == 'number' then
		M.tbl_print(M.tbl_cut_depth(x, { depth = y }))
	end
end

-- TODO: add hot reload with keybind and configurable handlers for matches, ideally use findMatcher
-- by default look for config/default.*/

return M
