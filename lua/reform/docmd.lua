---@type reform.docmd
---@diagnostic disable-next-line: missing-fields
local M = {
	defaults = {
		defaults = {
			convert = vim.lsp.util.convert_input_to_markdown_lines,
			stylize = vim.lsp.util.stylize_markdown,
			convert_sig = vim.lsp.util.convert_signature_help_to_markdown_lines,
		},
		overrides = {},
		config = {
			overrides = {
				convert = true,
				stylize = true,
				convert_sig = true,
				cmp_doc = true,
				cmp_sig = true,
			},
			ft = true,
			debug = false,
		},
	},
}
M.config = M.defaults.config

function M.defaults.overrides.convert(doc, _)
	if doc.value and #doc.value == 0 or not doc.value and #doc == 0 then return {} end
	if type(doc) == 'string' or doc.kind == 'plaintext' then
		return vim.split(doc.value or doc, '\n')
	end
	local str = doc.value
	if doc[1] and not str then
		str = {}
		for _, v in ipairs(doc) do
			if type(v[1]) == 'string' then
				if #str == 0 then
					str = v
				else
					for i = 1, #v do
						str[#str + 1] = v[i]
					end
				end
			else
				str[#str + 1] = v.value or v
			end
		end
		str = table.concat(str, '\n')
	end

	if M.config.ft == true or M.config.ft[vim.bo.filetype] == true then
		if M.config.debug then
			local f = io.open(M.config.debug, 'a+')
			if not f:read '*l' then
				f:write(str)
				f:close()
				f = require 'reform.formatter'(str, vim.bo.filetype)
				io.open(M.config.debug, 'w'):close()
				return f
			else
				f:close()
			end
		end
		return require 'reform.formatter'(str, vim.bo.filetype)
	elseif type(M.config.ft) == 'table' and type(M.config.ft[vim.bo.filetype]) == 'function' then
		M.config.ft[vim.bo.filetype](str, vim.bo.filetype)
	end

	-- regex fallback that does some basic transformation
	return vim.split(
		str
			:gsub('%s+(```\n)', '%1')
			:gsub('([ \n\t])`([^`\n]+%s[^`\n]+)`%s*', ('%1\n```%s\n%2```\n'):format(vim.bo.filetype)),
		'\n'
	)
end

function M.defaults.overrides.stylize(buf, contents, _)
	vim.bo[buf].ft = 'markdown'
	vim.api.nvim_buf_set_lines(buf, 0, -1, false, contents)
	return contents
end

function M.defaults.overrides.convert_sig(sig, ft, _)
	local p = sig.activeParameter
	-- intentionaly not testing activeSignature range for finding bad lsps
	sig = sig.signatures[(sig.activeSignature or -1) + 1] or sig.signatures[1]
	p = sig.parameters[(sig.activeParameter or p or -1) + 1] -- -1 for signature before arg section

	if p then -- mark the active parameter position before conversion
		local l = sig.label
		local s, e
		if type(p.label) == 'string' then
			s, e = l:find(p.label, 0, true)
			s = s - 1
		else
			s, e = unpack(p.label)
		end
		sig.label = table.concat({ l:sub(1, s), l:sub(s + 1, e), l:sub(e + 1) }, '___')
	end

	local ret = { ('```%s\n%s```'):format(ft, sig.label) }
	if p and p.documentation then ret[#ret + 1] = p.documentation.value or p.documentation end
	if sig.documentation then ret[#ret + 1] = sig.documentation.value or sig.documentation end
	ret = vim.lsp.util.convert_input_to_markdown_lines {
		kind = 'markdown',
		value = table.concat(ret, '\n\n'),
	}

	if p then -- determine the active parameter position after conversion
		local s = ret[2]:find '___'
		ret[2] = ret[2]:gsub('___', '', 1)
		local e = ret[2]:find '___'
		ret[2] = ret[2]:gsub('___', '', 1)
		return ret, { s - 1, e - 1 }
	end

	return ret
end

function M.defaults.overrides.cmp_doc(self)
	local item = self:get_completion_item()
	if not item.documentation then return {} end
	return vim.lsp.util.convert_input_to_markdown_lines(item.documentation)
end

function M.defaults.overrides.cmp_sig(self, sig, idx)
	local docs = {}
	if sig.label then docs[1] = ('```\n%s```'):format(self:_signature_label(sig, idx)) end
	local p = sig.parameters[idx]
	if p and p.documentation then docs[#docs + 1] = p.documentation.value or p.documentation end
	if sig.documentation then docs[#docs + 1] = sig.documentation.value or sig.documentation end

	return { kind = 'markdown', value = table.concat(docs, '\n\n') }
end

local set = {}
function set.convert(fn) vim.lsp.util.convert_input_to_markdown_lines = fn end
function set.stylize(fn) vim.lsp.util.stylize_markdown = fn end
function set.convert_sig(fn) vim.lsp.util.convert_signature_help_to_markdown_lines = fn end

local function onLoad(pkg, cb)
	if package.loaded[pkg] then
		cb(package.loaded[pkg])
	else
		package.preload[pkg] = function()
			package.preload[pkg] = nil
			for _, loader in pairs(package.loaders) do
				local ret = loader(pkg)
				if type(ret) == 'function' then
					local mod = ret()
					cb(mod)
					return mod
				end
			end
		end
	end
end

function set.cmp_doc(fn)
	onLoad('cmp.entry', function(pkg)
		M.defaults.overrides.cmp_doc = pkg.get_documentation
		pkg.get_documentation = fn
	end)
end
function set.cmp_sig(fn)
	onLoad('cmp_nvim_lsp_signature_help', function(pkg)
		M.defaults.overrides.cmp_sig = pkg._docs
		pkg._docs = fn
	end)
end

function M.setup(config)
	if type(config) == 'boolean' then
		if config then
			config = M.defaults.config
		else
			config = {}
			for k, _ in pairs(M.defaults.config.overrides) do
				config[k] = false
			end
			config = { overrides = config }
		end
	end

	M.config = vim.tbl_deep_extend('force', M.config, config)
	for k, v in pairs(M.config.overrides) do
		if v then
			set[k](type(v) == 'function' and v or M.defaults.overrides[k])
		else
			set[k](M.defaults.defaults[k])
		end
	end
end

return M
