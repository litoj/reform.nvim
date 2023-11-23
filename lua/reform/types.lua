---@alias reform.Overridable function|boolean defines which function to use - default/plugin default/provided

---@class reform.docmd.Overrides map of available lspdocs-related replacements and their settings
---@field convert? reform.Overridable overrides main lspdocs-to-markdown conversion (vim.lsp.util.convert_input_to_markdown_lines)
---@field stylize? reform.Overridable overrides docs-display buffer highlighting (vim.lsp.util.stylize_markdown)
---@field convert_sig? reform.Overridable overrides signature-help docs composition (vim.lsp.util.convert_signature_help_to_markdown_lines)
---@field cmp_doc? reform.Overridable overrides cmp preview docs parsing ('cmp.entry'.get_documentation)
---@field cmp_sig? reform.Overridable overrides cmp signature help docs parsing ('cmp_nvim_lsp_signature_help'._docs)

---@class reform.docmd.Config
---@field overrides? reform.docmd.Overrides lspdocs-related replacements
---@field ft? table<string,boolean|fun(string,vim.bo.ft):string[]>|true `ft` for which docs should be reformed (if supported)
---@field debug? string|false path for writing docs before processing

---@class reform.docmd.Defaults
---@field config reform.docmd.Config
---@field overrides reform.docmd.Overrides reform.nvim replacements of corresponding functions
---@field defaults reform.docmd.Overrides original functions for restoring default behaviour

---@class reform.docmd
---@field defaults reform.docmd.Defaults
---@field config reform.docmd.Config
---@field setup fun(config:boolean|reform.docmd.Config) opt-out of filetypes or set custom overrides

---@alias reform.WinConfig vim.api.keyset.float_config|{winhl:table<string,string>}
---@class reform.input.Config
---@field window? reform.WinConfig
---@field keymaps? {histPrev:string[],histNext:string[],confirm:string[],cancel:string[]} keymaps for history navigation (insert mode)

---@alias vim.ui.input function

---@class reform.input
---@field default vim.ui.input
---@field config reform.input.Config
---@field override vim.ui.input override with floating window and history navigation
---@field setup fun(config:reform.Overridable|reform.input.Config) customize window position/title/highlighting and history keybinds or use a custom implementation

---@alias vim.ui.select function

---@class reform.select
---@field default vim.ui.select
---@field config reform.WinConfig
---@field override vim.ui.select override with floating window and quick-select keybinds
---@field setup fun(config:reform.Overridable|reform.WinConfig) customize window position/title/highlighting or use a custom implementation

---@alias nvim.Keymap {[1]:string|string[], [2]: string} mode and key for vim.keymap.set
---@class reform.open_link.Config
---@field mappings? nvim.Keymap[] mappings for opening links or use reform.open_link.key(), reform.open_link.mouse(), or reform.open_link(line, col) directly
---@field handlers? {[1]:string,[2]:fun(path:string)}[] regex & matched link handler pairs, use defaults.handlers[] to pick and order the default handlers

---@class reform.open_link
---@overload fun(line:string, col:number) use setup handlers to open link at line:col
---@field defaults reform.open_link.Config
---@field config reform.open_link.Config
---@field key fun() open link under caret
---@field mouse fun() open link under mouse
---@field setup fun(config:reform.open_link.Config) set your custom shortcuts or disable the feature
---@field open_link fun(line:string, col:number) use setup handlers to open link at line:col

---@class reform.Config
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field input? reform.Overridable|reform.input.Config reform of vim.ui.input
---@field select? reform.Overridable|reform.WinConfig reform of vim.ui.select
---@field open_link? boolean|reform.open_link.Config reform of openable links - set your custom shortcuts and link detection
---@field man? boolean enable custom manpage formatting (using parser(bash))

---@class reform
---@field defaults reform.Config
---@field config reform.Config
---@field docmd reform.docmd lspdocs formatting to concise markdown structure
---@field input reform.input vim.ui.input with history navigation in a floating window
---@field select reform.select vim.ui.select with a floating window and quick selection keybinds
---@field open_link reform.open_link open links in the buffer or in the browser like in any other IDE
---@field man {setup:fun(enable:boolean)} manpage formatting using parser(bash)
---@field setup fun(config:reform.Config) setup reform.nvim just the way you like it
