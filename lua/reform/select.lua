local M = {
	default = vim.ui.select,
	---@type reform.winConfig
	config = {
		title_pos = 'center',
		title = { { '[', 'FloatBorder' }, { '', 'FloatTitle' }, { ']', 'FloatBorder' } },
		relative = 'cursor',
		border = 'rounded',
		col = -2,
		row = 1,
	},
}

function M.override(items, opts, on_choice)
	opts.prompt = opts.prompt and opts.prompt:gsub(': *$', '') or 'Select one of'
	opts.format_item = opts.format_item or tostring
	local callback
	local buf = vim.api.nvim_create_buf(false, true)
	vim.bo[buf].filetype = 'ui-select'
	local width = #opts.prompt
	local lines = {}
	for i, item in ipairs(items) do
		lines[i] = '[' .. i .. '] ' .. opts.format_item(item)
		if #lines[i] > width then width = #lines[i] end
		vim.keymap.set('n', tostring(i), function() callback(i) end, { buffer = buf })
	end
	vim.api.nvim_buf_set_lines(buf, 0, -1, true, lines)
	vim.bo[buf].modifiable = false
	vim.cmd.stopinsert()

	local win = require('reform.util').mkWin(
		buf,
		vim.tbl_extend('force', { width = width, height = #items }, M.config),
		opts.prompt
	)
	vim.api.nvim_win_set_cursor(win, { 1, 1 })
	vim.api.nvim_create_autocmd('CursorMoved', {
		buffer = buf,
		callback = function()
			local cursor = vim.api.nvim_win_get_cursor(win)
			if cursor[2] ~= 1 then vim.api.nvim_win_set_cursor(win, { cursor[1], 1 }) end
		end,
	})
	vim.cmd.stopinsert()

	vim.cmd [[
		syn match SelectNum /-\?\d\+/
		syn match SelectId /\d\+/ contained 
		syn match SelectOpt /^\[\d\+\] / contains=SelectId
		syn region SelectString start=/"/ skip=/'/ end=/"/ contained
		syn region SelectString start=/'/ skip=/"/ end=/'/ contained
		syn match SelectVar /\w\+/ contained
		syn region SelectVarDelim start="`" end="`" contains=SelectVar
		hi def link SelectNum Number
		hi def link SelectId Repeat
		hi def link SelectOpt Delimiter
		hi def link SelectString String
		hi def link SelectVar Variable
		hi def link SelectVarDelim Delimiter
	]]

	callback = function(i)
		vim.api.nvim_win_close(win, true)
		on_choice(i and items[i], i)
	end
	vim.keymap.set('n', 'q', callback, { buffer = buf })
	vim.keymap.set('n', '<Esc>', callback, { buffer = buf })
	vim.keymap.set(
		'n',
		'<CR>',
		function() callback(vim.api.nvim_win_get_cursor(win)[1]) end,
		{ buffer = buf }
	)
end

---@param config reform.Overridable|reform.winConfig
return function(config)
	if config then
		if type(config) == 'function' then
			vim.ui.select = config
		else
			vim.ui.select = M.override
			if type(config) == 'table' then M.config = vim.tbl_deep_extend('force', M.config, config) end
		end
	else
		vim.ui.select = M.default
	end
	return M
end
