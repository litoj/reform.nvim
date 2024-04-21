---@diagnostic disable: param-type-mismatch, need-check-nil
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
		debug = false,
	},
}
M.config = M.default_config

function M.override.reform.convert(doc, contents)
	if contents and #contents > 0 and M.config.debug then
		vim.notify('reform.docmd.convert(): ' .. vim.inspect(contents))
	end
	if doc.value and #doc.value == 0 or not doc.value and #doc == 0 then return {} end
	if type(doc) == 'string' then return vim.split(doc.value or doc, '\n') end
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

	local ft = vim.bo.filetype
	if -- file preview (constrained guess)
		str:sub(1, 3) == '```'
		and str:sub(-4) == '\n```'
		and #str >= 2048 -- expect docs to not be too long
		and str:sub(4, 7) ~= ' man' -- bash manpage can be very long
		and str:find('\n```', 4, true) == #str - 3 -- no code blocks in between
	then
	elseif M.config.ft == true or type(M.config.ft) == 'table' and M.config.ft[ft] == true then
		if type(M.config.debug) == 'string' then
			local hasFile
			if M.config.debug:sub(1, 1) == '"' then
				vim.fn.setreg(M.config.debug:sub(2, 1), str)
			else
				local f = io.open(M.config.debug, 'a+')
				hasFile = f:read '*l'
				f:write(str)
				f:close()
			end

			local ret = require 'reform.formatter'(str, ft)

			if M.config.debug:sub(1, 1) == '"' then
				if #M.config.debug == 2 or M.config.debug:sub(3, 3) == M.config.debug:sub(2, 2) then
					vim.fn.setreg(
						M.config.debug:sub(2, 2),
						str .. '\n\n>>>\n\n' .. (ret and table.concat(ret, '\n') or 'nil')
					)
				else
					vim.fn.setreg(M.config.debug:sub(3, 3), (ret and table.concat(ret, '\n') or 'nil'))
				end
			else
				if not hasFile then
					io.open(M.config.debug, 'w'):close()
				else
					local f = io.open(M.config.debug, 'a')
					f:write '\nFMT>>>\n'
					f:write(table.concat(ret, '\n'))
					f:write '<<<FMT\n'
					f:close()
				end
			end

			if ret then return ret end
		elseif M.config.debug then
			vim.notify(str)
		end
		local ret = require 'reform.formatter'(str, ft)
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
		str = '```' .. (M.config.labels[label] or label) .. '\n' .. str:sub(to + 5)
	else
		label = M.config.labels[label]
		if label then str = '```' .. label .. '\n' .. str:sub(to + 1) end
	end
	return vim.split(str:gsub('\n```', '```'), '\n')
end

function M.default_config.override.stylize(buf, contents, _)
	vim.bo[buf].ft = 'markdown'
	vim.api.nvim_buf_set_lines(buf, 0, -1, false, contents)
	return contents
end

function M.default_config.override.convert_sig(sig, ft, _)
	local p = sig.activeParameter
	-- intentionaly not testing activeSignature range for finding bad lsps
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
			return ret, { s - 1, e - 1 }
		end
	end

	return ret
end

function M.override.reform.cmp_doc(self)
	local item = self:get_completion_item()
	if not item.documentation then return {} end
	return vim.lsp.util.convert_input_to_markdown_lines(item.documentation)
end

function M.default_config.override.cmp_sig(self, sig, idx)
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
