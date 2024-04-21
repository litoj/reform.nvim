---@type reform.sig_help
---@diagnostic disable-next-line: missing-fields
local M = {
	overrides = {
		set = {},
		vim = {
			lsp_sig = vim.lsp.handlers['textDocument/signatureHelp'],
			lsc_on_attach = false,
		},
	},
	defaults = {
		overrides = {},
		config = {
			max_line_offset = 5,
			max_column_offset = 20,
			ignore_width_above = 0.8,
			valid_modes = { i = true, s = true },
			require_active_param = false,
			auto_show = true,
			win_config = {
				border = 'rounded',
				close_events = { 'BufLeave', 'WinScrolled' },
			},
			overrides = {
				lsp_sig = true,
				lsc_on_attach = true,
			},
			mappings = { { 'i', '<C-S-Space>' } },
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
M.config = M.defaults.config

function M.win.close(self)
	if self.id < 0 then return end
	if vim.api.nvim_win_is_valid(self.id) then vim.api.nvim_win_close(self.id, false) end
	self.id = -1
end

function M.win.is_valid(self)
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

function M.signature.needs_update(self, sig, content_only)
	local s = sig.signatures[(sig.activeSignature or 0) + 1]
	local param_idx = s.activeParameter or sig.activeParameter or -1

	if
		(param_idx < 0 and M.config.require_active_param)
		or (
			content_only
			and self.label == s.label
			and self.param_idx == param_idx
			and self.idx == sig.activeSignature
		)
	then
		return false
	end

	self.label = s.label
	self.param_idx = param_idx
	self.idx = sig.activeSignature
	return true
end

function M.defaults.overrides.lsp_sig(_, sig, ctx, config)
	-- Ignore result since buffer changed. This happens for slow language servers.
	if vim.api.nvim_get_current_buf() ~= ctx.bufnr then return end

	local update, cursor = M.win:is_valid()
	if not update then M.win:close() end

	if
		not (sig and sig.signatures and sig.signatures[1])
		or not M.config.valid_modes[vim.api.nvim_get_mode().mode]
		or not M.signature:needs_update(sig, update)
	then
		return
	end

	local lines, hl =
		vim.lsp.util.convert_signature_help_to_markdown_lines(sig, vim.bo[ctx.bufnr].filetype, {})
	if not lines or #lines == 0 then return end

	if update then
		vim.api.nvim_buf_set_lines(M.win.bufnr, 0, -1, false, lines)
	else
		if not cursor then cursor = vim.api.nvim_win_get_cursor(0) end
		M.win.cul, M.win.cuc = cursor[1], cursor[2]
		config = vim.tbl_deep_extend('force', M.config.win_config, config or {})
		config.max_height = config.max_height or math.floor(vim.api.nvim_win_get_height(0) / 3)
		config.focus_id = ctx.method
		M.win.bufnr, M.win.id = vim.lsp.util.open_floating_preview(lines, 'markdown', config)
		vim.bo[M.win.bufnr].modifiable = true
	end

	if hl then
		vim.api.nvim_buf_add_highlight(M.win.bufnr, -1, 'LspSignatureActiveParameter', 1, unpack(hl))
	end
end

function M.defaults.overrides.lsc_on_attach(client, bufnr)
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
							if vim.api.nvim_get_mode().mode == state.match:sub(3, 3) then M.win:close() end
						end)
					end
					return
				elseif M.config.auto_show then -- CursorHoldI or CompleteDone
					vim.lsp.buf.signature_help()
				end
			end,
			buffer = bufnr,
		})
	end
end

function M.toggle()
	local isValid = vim.api.nvim_win_is_valid(M.win.id)
	if isValid == M.config.auto_show then
		if M.config.auto_show then
			M.win:close()
			M.config.auto_show = nil -- to not display the window until new info
		else
			vim.lsp.buf.signature_help()
		end
	else
		M.config.auto_show = isValid
		vim.print('reform.signature.auto_show: ' .. tostring(M.config.auto_show))
	end
end

function M.overrides.set.lsp_sig(fn) vim.lsp.handlers['textDocument/signatureHelp'] = fn end
function M.overrides.set.lsc_on_attach(fn)
	if M.lsc_on_attach ~= nil then
		M.lsc_on_attach = fn
		return
	end
	M.lsc_on_attach = fn

	require('reform.util').with_mod('lspconfig.util', function(lsu)
		lsu.on_setup = lsu.add_hook_before(lsu.on_setup, function(config)
			config.on_attach = lsu.add_hook_before(config.on_attach, function(...)
				if M.lsc_on_attach then M.lsc_on_attach(...) end
			end)
		end)
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
			M.overrides.set[k](type(v) == 'function' and v or M.defaults.overrides[k])
		else
			M.overrides.set[k](M.overrides.vim[k])
		end
	end
	for _, bind in ipairs(M.config.mappings) do
		vim.keymap.set(bind[1], bind[2], M.toggle)
	end
end

return M
