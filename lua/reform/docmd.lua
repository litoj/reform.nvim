local M = {
	default = {
		convert = vim.lsp.util.convert_input_to_markdown_lines,
		stylize = vim.lsp.util.stylize_markdown,
		config = {
			override = {convert = true, stylize = true, cmp_doc = true},
			ft = {c = true, cpp = true, lua = true, java = true},
		},
	},
	override = {},
	set = {},
}

function M.override.convert(doc, _contents)
	-- vim.api.nvim_echo({{vim.inspect(doc)}}, false, {})
	if doc.value and #doc.value == 0 or not doc.value and #doc == 0 then return {} end
	if type(doc) == "string" or doc.kind == "plaintext" then return vim.split(doc.value or doc, "\n") end
	local str = doc.value
	if doc[1] and not str then
		str = ""
		for _, v in ipairs(doc) do
			if type(v[1]) == "string" then
				for _, s in ipairs(v) do str = str .. "\n" .. s end
			else
				str = str .. "\n" .. (v.value or v)
			end
		end
	end

	if M.config.ft[vim.bo.filetype] then
		-- vim.api.nvim_echo({{vim.inspect(str)}}, false, {})
		return require 'reform.docfmt'(str, vim.bo.filetype)
	end

	-- regex fallback that does some basic transformation
	return vim.split(str:gsub("%s+(```\n)", "%1"):gsub("([ \n\t])`([^`\n]+%s[^`\n]+)`%s*",
			"%1\n```" .. vim.bo.filetype .. "\n%2```\n"), "\n")
end

function M.override.stylize(buf, contents, _opts)
	vim.api.nvim_buf_set_option(buf, "filetype", "markdown")
	vim.api.nvim_buf_set_lines(buf, 0, 0, false, contents)
	return contents
end

function M.override.cmp_doc(self)
	local item = self:get_completion_item()
	if not item.documentation then return {} end
	return vim.lsp.util.convert_input_to_markdown_lines(item.documentation)
end

function M.set.convert(fn) vim.lsp.util.convert_input_to_markdown_lines = fn end
function M.set.stylize(fn) vim.lsp.util.stylize_markdown = fn end
function M.set.cmp_doc(fn)
	if package.loaded["cmp.entry"] then
		M.default.cmp_doc = package.loaded["cmp.entry"].get_documentation
		package.loaded["cmp.entry"].get_documentation = fn or M.default.cmp_doc
	else
		package.preload["cmp.entry"] = function()
			package.preload["cmp.entry"] = nil
			for _, loader in pairs(package.loaders) do
				local ret = loader("cmp.entry")
				if type(ret) == "function" then
					local mod = ret()
					M.default.cmp_doc = mod.get_documentation
					mod.get_documentation = fn or M.default.cmp_doc
					return mod
				end
			end
		end
	end
end

return function(config)
	if type(config) == "boolean" then
		config = config and {} or {override = {convert = false, stylize = false, cmp_doc = false}}
	end
	M.config = vim.tbl_deep_extend("force", M.default.config, config)
	for k, v in pairs(M.config.override) do
		if v then
			M.set[k](type(v) == "function" and v or M.override[k])
		else
			M.set[k](M.default[k])
		end
	end
	return M
end
