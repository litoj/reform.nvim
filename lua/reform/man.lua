local M = { config = false }

---@param config boolean enable custom manpage formatting
function M.setup(config)
	if M.config == config then return end
	M.config = config
	if M.config then
		if not M.group then M.group = vim.api.nvim_create_augroup('reform.man', {}) end
		vim.api.nvim_create_autocmd('FileType', {
			group = M.group,
			pattern = 'man',
			callback = function(state)
				vim.bo[state.buf].ft = 'markdown'
				local opt = vim.b[state.buf]
				if opt.reforman then return end
				opt.reforman = true
				vim.bo[state.buf].modifiable = true
				local input = table
					.concat(vim.api.nvim_buf_get_lines(state.buf, 0, -1, false), '\n')
					:gsub('\x1b%[[0-9;]*[A-Za-z]', '')
					:gsub('^\n*[^\n]+\n', '``` man')
					:gsub('\n*[^\n]+\n$', '\n\n```')
				vim.api.nvim_buf_set_lines(
					state.buf,
					0,
					-1,
					false,
					require 'reform.formatter'(input, 'bash')
				)
			end,
		})
	else
		if M.group then vim.api.nvim_clear_autocmds { group = M.group } end
	end
end

return M
