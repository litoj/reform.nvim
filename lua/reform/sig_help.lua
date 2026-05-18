---@type reform.sig_help
---@diagnostic disable-next-line: missing-fields
local M = {
	override = {
		set = {},
		vim = {
			lsp_sig = vim.lsp.buf.signature_help,
			lsc_on_attach = false,
		},
		reform = {},
	},
	default_config = {
		max_line_offset = 5,
		max_column_offset = 20,
		ignore_width_above = 0.8,
		valid_modes = { i = true, s = true },
		require_active_param = false, -- should we hide sighelp if the name of the fn is in focus
		auto_show = true,
		win = {
			border = 'rounded',
			close_events = { 'BufLeave', 'WinScrolled' },
		},
		override = {
			lsp_sig = true,
			lsc_on_attach = true,
		},
		mapping = {
			cycle_or_toggle_autoshow = { 'i', '<C-S-Space>' },
			toggle_autoshow = { 'n', '<C-S-Space>' },
		},
	},
	win = {
		bufnr = 0,
		id = -1,
		from_line = 0, -- adjusted to main window scroll (lines are 0-based)
		to_line = 1, -- from_line + height + 1
		width = 0,
		cul = 0,
		cuc = 1,
	},
	signature = {
		idx = -1,
		label = '',
		param_idx = -1,
	},
}
M.config = M.default_config

function M.win:close()
	if self.id < 0 then return end
	if vim.api.nvim_win_is_valid(self.id) then vim.api.nvim_win_close(self.id, false) end
	self.id = -1
end

function M.win:is_valid()
	if self.id < 0 or not vim.api.nvim_win_is_valid(self.id) then return false end
	local from_line = vim.api.nvim_win_get_position(self.id)[1] + vim.fn.line 'w0' - 1
	local to_line = from_line + vim.api.nvim_win_get_height(self.id) + 1
	local width = vim.api.nvim_win_get_width(0)
	local cursor = vim.api.nvim_win_get_cursor(0)

	return (cursor[1] < from_line or to_line < cursor[1]) -- cursor outside window
		and math.abs(self.cul - cursor[1]) < M.config.max_line_offset
		and (
			math.abs(self.cuc - cursor[2]) < width / 5
			or vim.api.nvim_win_get_width(self.id)
				> ( --
					M.config.ignore_width_above > 1 and M.config.ignore_width_above
					or width * M.config.ignore_width_above
				)
		),
		cursor
end

function M.signature:needs_update(result, content_only, advance)
	local active = (result.activeSignature or 0) + 1
	local sigs = result.signatures
	if advance then -- update index as the user requests
		active = (self.idx + advance - 1) % #sigs + 1
	else -- if on the same method, update the signature index to use the last one
		local old = sigs[self.idx]
		-- use old signature if it covers current param
		if
			old
			and old.label == self.label
			and #old.parameters > (old.activeParameter or result.activeParameter or -1)
		then
			active = self.idx
		end
	end

	local s = sigs[active]
	if not s then return false end
	local param_idx = s.activeParameter or result.activeParameter or -1

	if param_idx < 0 and M.config.require_active_param then return false end
	if
		content_only
		and self.label == s.label
		and self.param_idx == param_idx
		and self.idx == active
	then
		return false
	end

	self.label = s.label
	self.param_idx = param_idx
	self.idx = active
	result.activeSignature = active - 1
	return active
end

function M.override.reform.lsc_on_attach(client, bufnr)
	if client.server_capabilities.signatureHelpProvider then
		vim.api.nvim_create_autocmd({ 'CursorHoldI', 'CompleteDone', 'CursorMovedI', 'ModeChanged' }, {
			callback = function(state)
				if state.event == 'CursorMovedI' then
					if not M.win:is_valid() then M.win:close() end
					if M.config.auto_show == nil then M.config.auto_show = true end
					return
				elseif state.event == 'ModeChanged' then
					if not M.config.valid_modes[state.match:sub(3, 3)] then
						vim.schedule(function() -- delay for detecting snippet jumps
							if vim.fn.mode() == state.match:sub(3, 3) then M.win:close() end
						end)
					end
					return
				elseif M.config.auto_show then -- CursorHoldI or CompleteDone
					local pos = vim.api.nvim_win_get_cursor(0)
					if pos[1] == M.win.cul and pos[2] == M.win.cuc then return end
					vim.lsp.buf.signature_help()
				end
			end,
			buffer = bufnr,
		})
	end
end

local oid
---@param config? vim.lsp.buf.signature_help.Opts|{advance:integer} extended config for advancing sigs
function M.override.reform.lsp_sig(config)
	local win = vim.api.nvim_get_current_win()
	vim.lsp.buf_request_all(
		0,
		'textDocument/signatureHelp',
		function(client) return vim.lsp.util.make_position_params(win, client.offset_encoding) end,
		function(results, ctx)
			config = config or {}
			-- Ignore result since buffer changed. This happens for slow language servers.
			if vim.api.nvim_get_current_buf() ~= ctx.bufnr then return end

			local update, cursor = M.win:is_valid()
			if not update then M.win:close() end

			local res ---@type lsp.SignatureHelp
			for _, r in pairs(results) do
				if not r.err and r.result and r.result.signatures then
					res = r.result
					break
				end
			end

			if not res or not res.signatures[1] then return end
			if not M.config.valid_modes[vim.api.nvim_get_mode().mode] then return end

			-- already up to date
			local active = M.signature:needs_update(res, update, config.advance)
			if not active then return end

			local lines, hl =
				vim.lsp.util.convert_signature_help_to_markdown_lines(res, vim.bo[ctx.bufnr].filetype, {})
			if not lines or #lines == 0 then return end

			if update then -- update window to new signature
				vim.api.nvim_buf_set_lines(M.win.bufnr, 0, -1, false, lines)
				vim.api.nvim_win_set_config(M.win.id, {
					title = tostring(active),
				})
			else -- recreate the window at a new position and with possibly new content
				if not cursor then cursor = vim.api.nvim_win_get_cursor(0) end
				M.win.cul, M.win.cuc = cursor[1], cursor[2]
				config = vim.tbl_deep_extend('force', M.config.win, config)
				config.max_height = config.max_height or math.floor(vim.api.nvim_win_get_height(0) / 3)
				config.focus_id = ctx.method
				config.title = tostring(active)
				M.win.bufnr, M.win.id = vim.lsp.util.open_floating_preview(lines, 'markdown', config)
				vim.bo[M.win.bufnr].modifiable = true
			end

			if hl then
				vim.hl.range(
					M.win.bufnr,
					1,
					'LspSignatureActiveParameter',
					{ hl[1], hl[2] },
					{ hl[3], hl[4] }
				)
			end
		end
	)
end

function M.show_or_cycle()
	local isValid = vim.api.nvim_win_is_valid(M.win.id)
	vim.lsp.buf.signature_help { advance = isValid and 1 or nil }
end

function M.cycle_or_toggle_autoshow()
	local isValid = vim.api.nvim_win_is_valid(M.win.id)
	vim.lsp.buf.signature_help { advance = isValid and 1 or nil }
	if isValid and M.signature.idx == 1 then
		M.win:close()
		M.toggle_autoshow()
	end
end

function M.toggle_autoshow()
	M.config.auto_show = M.config.auto_show == false
	vim.print('reform.sig_help.auto_show: ' .. tostring(M.config.auto_show))
end

function M.show_or_toggle_autoshow()
	local isValid = vim.api.nvim_win_is_valid(M.win.id)
	if isValid == M.config.auto_show then
		if M.config.auto_show then
			M.win:close()
			M.config.auto_show = nil -- to not display the window until new info
		else
			vim.lsp.buf.signature_help()
		end
	else
		M.toggle_autoshow()
	end
end

function M.override.set.lsp_sig(fn) vim.lsp.buf.signature_help = fn end
function M.override.set.lsc_on_attach(fn)
	local ag = vim.api.nvim_create_augroup('sig_help', { clear = true })
	vim.api.nvim_create_autocmd('LspAttach', {
		group = ag,
		callback = function(s) fn(vim.lsp.get_client_by_id(s.data.client_id), s.buf) end,
	})
end

---@diagnostic disable-next-line: return-type-mismatch
function M.gen_mapping(_, action) return M[action] end

return M
