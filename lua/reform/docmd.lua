local M = {
	---@type reform.docmd.Defaults
	default = {
		convert = vim.lsp.util.convert_input_to_markdown_lines,
		stylize = vim.lsp.util.stylize_markdown,
		---@type reform.docmd.Config
		config = {
			override = { convert = true, stylize = true, cmp_doc = true, cmp_sig = true },
			ft = true,
		},
	},
	override = {},
	set = {},
}

function M.override.convert(doc, _)
	if doc.value and #doc.value == 0 or not doc.value and #doc == 0 then return {} end
	if type(doc) == 'string' or doc.kind == 'plaintext' then
		return vim.split(doc.value or doc, '\n')
	end
	local str = doc.value
	if doc[1] and not str then
		str = ''
		for _, v in ipairs(doc) do
			if type(v[1]) == 'string' then
				for _, s in ipairs(v) do
					str = str .. '\n' .. s
				end
			else
				str = str .. '\n' .. (v.value or v)
			end
		end
	end

	if M.config.ft == true or M.config.ft[vim.bo.filetype] == true then
		if M.config.debug then
			local f = io.open(M.config.debug, 'a')
			f:write('# >>>\n')
			f:write(str)
			f:write('\n')
			f:close()
		end
		return require 'reform.docfmt'(str, vim.bo.filetype)
	elseif type(M.config.ft) == 'table' and type(M.config.ft[vim.bo.filetype]) == 'function' then
		M.config.ft[vim.bo.filetype](str, vim.bo.filetype)
	end

	-- regex fallback that does some basic transformation
	return vim.split(
		str
			:gsub('%s+(```\n)', '%1')
			:gsub('([ \n\t])`([^`\n]+%s[^`\n]+)`%s*', '%1\n```' .. vim.bo.filetype .. '\n%2```\n'),
		'\n'
	)
end

function M.override.stylize(buf, contents, _)
	vim.bo[buf].ft = 'markdown'
	vim.api.nvim_buf_set_lines(buf, 0, -1, false, contents)
	return contents
end

function M.override.cmp_doc(self)
	local item = self:get_completion_item()
	if not item.documentation then return {} end
	return vim.lsp.util.convert_input_to_markdown_lines(item.documentation)
end

function M.override.cmp_sig(self, sig, idx)
	local docs = {}
	if sig.label then docs[1] = '```\n' .. self:_signature_label(sig, idx) .. '```' end
	local p = sig.parameters[idx]
	if p and p.documentation then docs[#docs + 1] = p.documentation.value end
	if sig.documentation then docs[#docs + 1] = sig.documentation.value end

	return { kind = 'markdown', value = table.concat(docs, '\n\n') }
end

function M.set.convert(fn) vim.lsp.util.convert_input_to_markdown_lines = fn end
function M.set.stylize(fn) vim.lsp.util.stylize_markdown = fn end

local function onLoad(pkg, fn)
	if package.loaded[pkg] then
		fn(package.loaded[pkg])
	else
		package.preload[pkg] = function()
			package.preload[pkg] = nil
			for _, loader in pairs(package.loaders) do
				local ret = loader(pkg)
				if type(ret) == 'function' then
					local mod = ret()
					fn(mod)
					return mod
				end
			end
		end
	end
end

function M.set.cmp_doc(fn)
	onLoad('cmp.entry', function(pkg)
		M.default.cmp_doc = pkg.get_documentation
		pkg.get_documentation = fn or M.default.cmp_doc
	end)
end
function M.set.cmp_sig(fn)
	onLoad('cmp_nvim_lsp_signature_help', function(pkg)
		M.default.cmp_sig = pkg._docs
		pkg._docs = fn or M.default.cmp_sig
	end)
end

---@param config boolean|reform.docmd.Config reform documentation to your liking
function M.setup(config)
	if type(config) == 'boolean' then
		config = config and {}
			or { override = { convert = false, stylize = false, cmp_doc = false, cmp_sig = false } }
	end
	M.config = vim.tbl_deep_extend('force', M.default.config, config)
	for k, v in pairs(M.config.override) do
		if v then
			M.set[k](type(v) == 'function' and v or M.override[k])
		else
			M.set[k](M.default[k])
		end
	end
end

return M
