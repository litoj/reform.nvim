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
---@field debug? string|boolean path for writing docs before processing or true for just pringing

---@class reform.docmd.Defaults
---@field config reform.docmd.Config
---@field overrides reform.docmd.Overrides reform.nvim replacements of corresponding functions
---@field vim reform.docmd.Overrides original functions for restoring default behaviour

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
---@alias reform.open_link.Event {buf:integer,line:integer,column:integer,mouse?:boolean}
---@alias reform.open_link.Handler {pattern:string,use:fun(match:string,ev:reform.open_link.Event):nil|1} 1 for failure

---@class reform.open_link.Config
---@field fallback? 'noop'|'definition'|{copy:boolean,print:boolean,branch:'current'|'default'|fun(ev:reform.open_link.Event):string}
---       - 'copy','print' the git link generated for current cursor line
---@field mappings? nvim.Keymap[] mappings for opening links or use reform.open_link.key(), reform.open_link.mouse()
---@field handlers? table<integer,reform.open_link.Handler|string> regex & matched link handler pairs, use `ol.handlers[]` to pick and order the default handlers - just by name

---@class reform.open_link
---@overload fun(line:string, col:number) use setup handlers to open link at line:col
---@field defaults reform.open_link.Config
---@field config reform.open_link.Config
---@field key fun() open link under caret
---@field mouse fun() open link under mouse - at click position
---@field handle fun(ev:reform.open_link.Event) use setup handlers to open link at line:col
---@field setup fun(config:reform.open_link.Config) set your custom shortcuts or disable the feature

---@class reform.Config
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field input? reform.Overridable|reform.input.Config reform of vim.ui.input
---@field select? reform.Overridable|reform.WinConfig reform of vim.ui.select
---@field open_link? boolean|reform.open_link.Config reform of openable links - set your custom shortcuts and link detection

---@class reform
---@field defaults reform.Config
---@field config reform.Config
---@field docmd reform.docmd lspdocs formatting to concise markdown structure
---@field input reform.input vim.ui.input with history navigation in a floating window
---@field select reform.select vim.ui.select with a floating window and quick selection keybinds
---@field open_link reform.open_link open links in the buffer or in the browser like in any other IDE
---@field setup fun(config:reform.Config) setup reform.nvim just the way you like it
