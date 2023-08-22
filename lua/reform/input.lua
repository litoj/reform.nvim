local M = {
	default = vim.ui.input,
	---@type reform.winConfig
	config = {
		title_pos = 'center',
		title = { { '[', 'FloatBorder' }, { '', 'FloatTitle' }, { ']', 'FloatBorder' } },
		relative = 'cursor',
		border = 'rounded',
		height = 1,
		row = -3,
	},
}

function M.override(opts, on_confirm)
	local default = opts.default or ''
	opts.prompt = opts.prompt and opts.prompt:gsub(': *$', '') or ''
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-input'
	local width = #opts.prompt > #default and #opts.prompt + 2 or #default
	local win = require('reform.util').mkWin(
		buf,
		vim.tbl_extend(
			'force',
			{ col = -width / 2, width = width + (#default > 10 and #default * 2 or 20) },
			M.config
		),
		opts.prompt
	)
	vim.wo[win].cursorline = false
	vim.api.nvim_set_current_line(default)
	vim.cmd.startinsert { bang = true }

	local typed = default
	local last = vim.fn.histnr '@'
	local histpos = last + 1
	local histUpdate = true

	local function callback(confirmed)
		local text = vim.api.nvim_get_current_line()
		vim.api.nvim_win_close(win, true)
		if confirmed then
			if #text > 3 then vim.fn.histadd('@', text) end
			on_confirm(text)
		else
			on_confirm(opts.cancelreturn == nil and '' or opts.cancelreturn)
		end
	end
	vim.keymap.set('i', '<C-q>', callback, { buffer = buf })
	vim.keymap.set('i', '<Esc>', callback, { buffer = buf })
	vim.keymap.set('i', '<CR>', function() callback(true) end, { buffer = buf })
	vim.keymap.set('i', '<Up>', function()
		if histpos == last + 1 then typed = vim.api.nvim_get_current_line() end
		for i = histpos - 1, 1, -1 do
			local str = vim.fn.histget('@', i)
			if str:find(typed, 1, true) then
				histpos = i
				histUpdate = true
				vim.api.nvim_set_current_line(str)
				return
			end
		end
	end, { buffer = buf })
	vim.keymap.set('i', '<Down>', function()
		if histpos == last + 1 then return end
		for i = histpos + 1, last do
			local str = vim.fn.histget('@', i)
			if str:find(typed, 1, true) then
				histpos = i
				histUpdate = true
				vim.api.nvim_set_current_line(str)
				return
			end
		end
		vim.api.nvim_set_current_line(typed)
	end, { buffer = buf })
	vim.api.nvim_create_autocmd('TextChangedI', {
		buffer = buf,
		callback = function()
			if not histUpdate then
				histpos = last + 1
			else
				histUpdate = false
			end
			if opts.highlight then opts.highlight(buf) end
		end,
	})
end

---@param config reform.Overridable|reform.winConfig
return function(config)
	if config then
		if type(config) == 'function' then
			vim.ui.input = config
		else
			vim.ui.input = M.override
			if type(config) == 'table' then M.config = vim.tbl_deep_extend('force', M.config, config) end
		end
	else
		vim.ui.input = M.default
	end
	return M
end
