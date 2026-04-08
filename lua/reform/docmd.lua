---@diagnostic disable: param-type-mismatch, need-check-nil, undefined-field
---@type reform.docmd
---@diagnostic disable-next-line: missing-fields
local M = {
	override = {
		set = {},
		vim = {
			convert = vim.lsp.util.convert_input_to_markdown_lines,
			stylize = vim.lsp.util.stylize_markdown,
			convert_sig = vim.lsp.util.convert_signature_help_to_markdown_lines,
		},
		reform = {},
	},
	default_config = {
		override = {
			convert = true,
			stylize = true,
			convert_sig = true,
			cmp_doc = true,
			cmp_sig = true,
		},
		labels = { cs = 'c_sharp' },
		ft = true, -- TODO: set to formatters → export fmt fn per lang all in table
		max_doc_len_increase = 500,
		debug = '/tmp/reform.dbg',
	},
}
M.config = M.default_config

---@param str string
---@return string[]
function M.convert(str)
	local ft = vim.bo.filetype
	if -- file preview (constrained guess)
		str:sub(1, 3) == '```'
		and str:sub(-4) == '\n```'
		and #str >= 2048 -- expect docs to not be too long
		and str:sub(4, 7) ~= ' man' -- bash manpage can be very long
		and str:find('\n```', 4, true) == #str - 3 -- no code blocks in between
	then
	elseif M.config.ft == true or type(M.config.ft) == 'table' and M.config.ft[ft] == true then
		if M.config.debug then
			if M.config.debug:sub(1, 1) == '"' then -- copy to register [2]
				vim.fn.setreg(M.config.debug:sub(2, 2), str)
			else -- write input to file
				if require('reform.util').exists(M.config.debug) then -- preserve
					M.config.debug = false
				else
					local f = io.open(M.config.debug, 'a+')
					f:write(str)
					f:close()
				end
			end
		end

		local ret = require 'reform.formatter'(str, ft, M.config.max_doc_len_increase)

		if M.config.debug then
			if M.config.debug:sub(1, 1) == '"' then --
				if #M.config.debug == 2 or M.config.debug:sub(3, 3) == M.config.debug:sub(2, 2) then
					vim.fn.setreg(
						M.config.debug:sub(2, 2),
						str .. '\n\n>>>\n\n' .. (ret and table.concat(ret, '\n') or 'nil')
					)
				else
					vim.fn.setreg(M.config.debug:sub(3, 3), (ret and table.concat(ret, '\n') or 'nil'))
				end
			elseif require('reform.util').exists(M.config.debug) then
				vim.fs.rm(M.config.debug) -- delete input file if nothing went wrong
			end
		end

		if ret then return ret end
	elseif type(M.config.ft) == 'table' and type(M.config.ft[ft]) == 'function' then
		return M.config.ft[ft](str, ft)
	end

	-- no handlers for this text → consider a file preview
	if str:sub(1, 3) ~= '```' then
		str = str:sub(1, 3) == '﻿' and str:sub(4) or str
		ft = ({ ['-'] = 'yaml', ['#'] = 'bash', ['<'] = 'xml', ['{'] = 'json' })[str:sub(1, 1)]
		if ft then str = string.format('```%s\n%s```', ft, str) end
		return vim.split(str, '\n')
	end
	local _, to, label = str:find('^(.-)\n', 4)
	if str:sub(to + 1, to + 3) == '﻿' then -- windows files cmp preview bug
		str = ('```%s\n%s'):format(M.config.labels[label] or label, str:sub(to + 5))
	else
		label = M.config.labels[label]
		if label then str = '```' .. label .. '\n' .. str:sub(to + 1) end
	end
	return vim.split(str, '\n')
end

---@param doc lsp.MarkedString|lsp.MarkedString[]|lsp.MarkupContent
---@param contents string[]? List of strings to extend with converted lines. Defaults to {}.
---@return string[] extended with lines of converted markdown.
---@see https://microsoft.github.io/language-server-protocol/specifications/specification-current/#textDocument_hover
function M.override.reform.convert(doc, contents)
	if doc.value then
		if #doc.value == 0 then return {} end
	else
		if #doc == 0 then return {} end
	end
	if type(doc) == 'string' then return vim.split(doc.value or doc, '\n') end

	local str = doc.value
	if doc[1] and not str then
		local tbl = {}
		for _, v in ipairs(doc) do
			if v[1] then
				for _, s in ipairs(v) do
					tbl[#tbl + 1] = s
				end
			else
				tbl[#tbl + 1] = v.value or v
			end
		end
		str = table.concat(tbl, '\n')
	end

	local ret = M.convert(str)
	if not contents then return ret end
	for _, v in ipairs(ret) do
		contents[#contents + 1] = v
	end
	return contents
end

function M.override.reform.stylize(buf, contents, _)
	vim.bo[buf].ft = 'markdown'
	vim.api.nvim_buf_set_lines(buf, 0, -1, false, contents)
	return contents
end

---@param sig lsp.SignatureHelp Response of `textDocument/SignatureHelp`
---@param ft string? filetype that will be use as the `lang` for the label markdown code block
---@param triggers string[]? list of trigger characters from the lsp server. used to better determine parameter offsets
---@return string[]? # lines of converted markdown.
---@return Range4? # highlight range for the active parameter
function M.override.reform.convert_sig(sig, ft, triggers)
	local p = sig.activeParameter
	-- NOTE: intentionaly not testing activeSignature range for finding bad lsps
	sig = sig.signatures[(sig.activeSignature or 0) + 1]
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

	local ret = { ('```%s\n%s\n```'):format(ft, sig.label) }
	if p and p.documentation then ret[#ret + 1] = p.documentation.value or p.documentation end
	if sig.documentation then ret[#ret + 1] = sig.documentation.value or sig.documentation end
	ret = vim.lsp.util.convert_input_to_markdown_lines {
		kind = 'markdown',
		value = table.concat(ret, '\n\n'),
	}

	if p then -- determine the active parameter position after conversion
		local s = ret[2]:find '___'
		if s then
			ret[2] = ret[2]:gsub('___', '', 1)
			local e = ret[2]:find '___'
			if not e then
				_, e = ret[2]:find('^[%w_]+%W', s)
			else
				ret[2] = ret[2]:gsub('___', '', 1)
			end
			return ret, { 1, s - 1, 1, e - 1 } -- start<line, col>, end<line, col>
		end
	end

	return ret
end

---@module 'blink.cmp'
--- Set it to completion.documentation.draw to use
---@param opts blink.cmp.CompletionDocumentationDrawOpts
function M.blink_doc(opts)
	if not opts.item.reformed then
		local text = opts.item.detail or opts.item.label
		if text:sub(1, 1) ~= '`' then text = ('```%s\n%s\n```'):format(vim.bo.filetype, text) end

		local docs = opts.item.documentation
		local val = docs and docs.value or docs
		if docs and #val > 0 then
			if docs.value then docs = { docs.value } end
			docs[1] = docs[1]:gsub('^%-%-%-', '')
			table.insert(docs, 1, text)
		else
			---@diagnostic disable-next-line: cast-local-type
			docs = { text }
		end

		opts.item.documentation = vim.lsp.util.convert_input_to_markdown_lines(docs)
		---@diagnostic disable-next-line: inject-field custom value for tracking already processed docs
		opts.item.reformed = true
	end

	local buf = opts.window.buf
	opts.window.config.filetype = 'markdown'
	vim.api.nvim_buf_set_lines(buf, 0, -1, true, opts.item.documentation)
end

function M.override.reform.cmp_doc(self)
	local item = self:get_completion_item()
	if not item.documentation then return {} end
	return vim.lsp.util.convert_input_to_markdown_lines(item.documentation)
end

function M.override.reform.cmp_sig(self, sig, idx)
	local docs = {}
	---@diagnostic disable-next-line: undefined-field
	if sig.label then docs[1] = ('```\n%s```'):format(self:_signature_label(sig, idx)) end
	local p = sig.parameters[idx]
	if p and p.documentation then docs[#docs + 1] = p.documentation.value or p.documentation end
	if sig.documentation then docs[#docs + 1] = sig.documentation.value or sig.documentation end

	return { kind = 'markdown', value = table.concat(docs, '\n\n') }
end

function M.override.set.convert(fn) vim.lsp.util.convert_input_to_markdown_lines = fn end
function M.override.set.stylize(fn) vim.lsp.util.stylize_markdown = fn end
function M.override.set.convert_sig(fn) vim.lsp.util.convert_signature_help_to_markdown_lines = fn end

function M.override.set.cmp_doc(fn)
	require('reform.util').with_mod('cmp.entry', function(pkg)
		M.default_config.override.cmp_doc = pkg.get_documentation
		pkg.get_documentation = fn
	end)
end
function M.override.set.cmp_sig(fn)
	require('reform.util').with_mod('cmp_nvim_lsp_signature_help', function(pkg)
		M.default_config.override.cmp_sig = pkg._docs
		pkg._docs = fn
	end)
end

return M
