local M = {
	default = {
		convert = vim.lsp.util.convert_input_to_markdown_lines,
		stylize = vim.lsp.util.stylize_markdown,
		config = {override = {}, ft = {c = true, cpp = true, lua = true}},
	},
	set = {},
}

function M.default.config.override.convert(doc, contents)
	-- vim.api.nvim_echo({{vim.inspect(doc)}}, false, {})
	if doc.value and #doc.value == 0 or not doc.value and #doc == 0 then return {} end
	if type(doc) == "string" or doc.kind == "plaintext" then return vim.split(doc.value or doc, "\n") end
	if doc.language then return vim.split("```" .. doc.language .. "\n" .. doc.value .. "```", "\n") end
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
		-- vim.api.nvim_echo({{str}}, false, {})
		str = require 'reform.docfmt.main'(str, vim.bo.filetype)
		-- vim.api.nvim_echo({{vim.inspect(str)}}, false, {})
		return vim.split(str, "\n")
	end

	str = str:gsub("%s+(```\n)", "%1"):gsub("([ \n\t])`([^`\n]+%s[^`\n]+)`%s*",
			"%1\n```" .. vim.bo.filetype .. "\n%2```\n")
	if vim.bo.filetype == "java" then
		str = str:gsub("{(%a+)}", "*`%1`*")
		local code = false
		for _, v in ipairs(vim.split(str, "\n")) do
			if #v > 4 then
				local _, idx = v:find("^>?    ")
				if idx and (code or not v:find(" +*", idx + 1)) then
					v = v:sub(idx + 1)
					if not code then
						contents[#contents + 1] = "```java"
						code = true
					end
				elseif code then
					code = false
					contents[#contents] = contents[#contents] .. "```"
				end
				contents[#contents + 1] = v;
			end
		end
	else
		return vim.split(str, "\n");
	end
end

function M.default.config.override.stylize(buf, contents, _opts)
	vim.api.nvim_buf_set_option(buf, "filetype", "markdown")
	vim.api.nvim_buf_set_lines(buf, 0, 0, false, contents)
	return contents
end

function M.default.config.override.cmp_doc(self)
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
			M.set[k](v)
		else
			M.set[k](M.default[k])
		end
	end
end
