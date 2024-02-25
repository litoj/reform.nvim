---@alias reform.util.WinConfig vim.api.keyset.float_config|{winhl:table<string,string>}
---@alias reform.util.Match {from:integer,to:integer,[integer]:string} all matched groups + bounds of the entire matched text
---@alias reform.util.MatcherSorting {order:integer,matcher:integer,offset:integer,length:integer}|fun(order:integer,matcher:reform.util.Matcher,match:reform.util.Match):integer
---@alias reform.util.Event {buf:integer,line:integer,column:integer,opts:{noFromCheck?:boolean,sorting?:reform.util.MatcherSorting,setCol?:fun(ev:reform.util.Event,match:reform.util.Match):integer|nil}}
---@alias reform.util.Matcher {luapat?:string,vimre?:string,weight?:integer,use:fun(match:string,info:reform.util.Match,ev:reform.util.Event):nil|false} returns false for failure
---@alias reform.util.MatcherMap table<string,reform.util.Matcher>
---@alias reform.util.MatcherRefs (reform.util.Matcher|string)[] matchers or their references by name
---@alias reform.util.MatcherList reform.util.MatcherRefs|fun(event:reform.util.Event):reform.util.MatcherRefs regex & matched text handler pairs or reference to predefined matchers
---@class reform.util
---@field winConfig reform.util.WinConfig default window configuration
---@field mkWin fun(buf:integer,opts:reform.util.WinConfig,prompt:string): integer returns window id
---@field findMatch fun(event:reform.util.Event,matchers:reform.util.MatcherList,knownHandlers:reform.util.MatcherMap,sorting:reform.util.MatcherSorting):boolean

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

---@class reform.input.Config
---@field window? reform.util.WinConfig
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
---@field config reform.util.WinConfig
---@field override vim.ui.select override with floating window and quick-select keybinds
---@field setup fun(config:reform.Overridable|reform.util.WinConfig) customize window position/title/highlighting or use a custom implementation

---@alias nvim.Keymap {[1]:string|string[], [2]: string} mode and key for vim.keymap.set

---@class reform.link.Config
---@field fallback? 'noop'|'definition'|{copy:boolean,print:boolean,branch:'current'|'default'|fun(ev:reform.util.Event):string}
---       - 'copy','print' the git link generated for current cursor line
---@field mappings? nvim.Keymap[] mappings for opening links or use reform.ink.key(), reform.link.mouse()
---@field matchers? reform.util.MatcherList

---@class reform.link
---@field defaults reform.link.Config
---@field config reform.link.Config
---@field key fun() open link under caret
---@field mouse fun() open link under mouse - at click position
---@field handle fun(ev:reform.util.Event) use setup matchers to open link at line:col
---@field setup fun(config:reform.link.Config) set your custom shortcuts+actions or disable the feature

---@class reform.toggle.Config
---@field mappings? {[1]:string|string[], [2]:string, [3]:{action:'tgl'|'inc'|'dec',afterCursor?:boolean,moveCursor?:boolean}}[] mappings for actions - mode, keybind, action
---@field matchers? reform.util.MatcherList
---@field sorting? reform.util.MatcherSorting

---@class reform.toggle
---@field defaults reform.toggle.Config
---@field config reform.toggle.Config
---@field genBind fun(evCfg:reform.util.Event) generate binding with preconfigured event
---@field handle fun(ev:reform.util.Event) use setup matchers to change value at line:col
---@field setup fun(config:reform.link.Config) set your custom shortcuts+actions or disable the feature

---@class reform.Config
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field input? reform.Overridable|reform.input.Config reform of vim.ui.input
---@field select? reform.Overridable|reform.util.WinConfig reform of vim.ui.select
---@field link? boolean|reform.link.Config reform of openable links - customizable link detection and actions
---@field toggle? boolean|reform.toggle.Config custom value toggler/changer system - inc/dec/tgl easily

---@class reform
---@field defaults reform.Config
---@field config reform.Config
---@field setup fun(config:reform.Config) setup reform.nvim just the way you like it
