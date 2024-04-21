---@alias reform.util.WinConfig vim.api.keyset.float_config|{winhl:table<string,string>}
---@alias reform.util.Match {from:integer,to:integer,[integer]:string} all matched groups + bounds of the entire matched text
---@class reform.util.MatchFilter
---@field tolerance? {startPost:integer,startPre:integer,endPost:integer,endPre:integer} startPost=if match starts after cursor, how far after (post-cursor) it can start...
---@field sorting? {order:integer,matcher:integer,offset:integer,length:integer}|fun(ev:reform.util.Event,order:integer,matcher:reform.util.Matcher,match:reform.util.Match):integer|false
---@alias reform.util.Event {buf:integer,line:integer,column:integer,filter?:reform.util.MatchFilter}
---@alias reform.util.Matcher {luapat?:string,vimre?:string,group?:integer,use:fun(match:string,info:reform.util.Match,ev:reform.util.Event):nil|false} returns false for failure, group= lower → higher priority
---@alias reform.util.MatcherMap table<string,reform.util.Matcher>
---@alias reform.util.MatcherRefs (reform.util.Matcher|string)[] matchers or their references by name
---@alias reform.util.MatcherList reform.util.MatcherRefs|fun(event:reform.util.Event):reform.util.MatcherRefs regex & matched text handler pairs or reference to predefined matchers
---@class reform.util
---@field winConfig reform.util.WinConfig default window configuration
---@field filter reform.util.MatchFilter default findMatch filter/settings - tolerance + sorting
---@field mkWin fun(buf:integer,opts:reform.util.WinConfig,prompt:string): integer returns window id
---@field findMatch fun(event:reform.util.Event,matchers:reform.util.MatcherList,knownHandlers:reform.util.MatcherMap,filter:reform.util.MatchFilter):reform.util.Match|false
---@field applyMatcher fun(matcher:reform.util.Matcher,event?:reform.util.Event):reform.util.Match|false
---@field with_mod fun(mod: string, callback:function) run callback when given module is loaded

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
---@field labels? table<string,string> map of corrections for markdown filetype labels (' man' → bash)

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
---@field defaults reform.input.Config
---@field config reform.input.Config
---@field override vim.ui.input override with floating window and history navigation
---@field setup fun(config:reform.Overridable|reform.input.Config) customize window position/title/highlighting and history keybinds or use a custom implementation

---@alias vim.ui.select function

---@class reform.select
---@field default vim.ui.select
---@field defaults reform.util.WinConfig
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
---@field filter? reform.util.MatchFilter

---@class reform.toggle
---@field defaults reform.toggle.Config
---@field config reform.toggle.Config
---@field genBind fun(evCfg:reform.util.Event) generate binding with preconfigured event
---@field handle fun(ev:reform.util.Event) use setup matchers to change value at line:col
---@field setup fun(config:reform.link.Config) set your custom shortcuts+actions or disable the feature

---@class reform.tbl_extras.Config
---@field diff? {expand_unique?:any} other tables for the field are `nil` → copy or use custom value
---@field cut_depth? {depth?:integer,cuts?:any} depth to cut off nested tables, replacement for cut-off tables (false to disable)
---@field global_print? boolean set custom table printer as extension of the global one

---@class reform.tbl_extras
---@field defaults reform.tbl_extras.Config
---@field config reform.tbl_extras.Config
---@field tbl_diff fun(opts:{expand_unique?:any},src:table,...:table):table? create diff of table contents compared to src
---@field tbl_short_diff fun(src:table,...:table):table? diff without unique tables expansion
---@field tbl_full_diff fun(src:table,...:table):table? diff with unique tables expansion
---@field tbl_cut_depth fun(tbl:table,opts?:{depth?:integer,cuts?:any}):table cut table at said depth
---@field tbl_print fun(tbl:table) print table with guarantee of visibility to the user
---@field print fun(...) global printer extension for tables - 1 tbl=print with default depth, 2+ tbls=print with diff to first
---@field setup fun(config:reform.tbl_extras.Config) set your custom table diff/cut/print settings

---@class reform.sig_help.Overrides
---@field lsp_sig? reform.Overridable
---@field lsc_on_attach? reform.Overridable

---@class reform.sig_help.Config
---@field max_line_offset? integer
---@field max_column_offset? integer
---@field ignore_width_above? float
---@field valid_modes? table<string,boolean>
---@field require_active_param? boolean
---@field auto_show? boolean
---@field win_config? reform.util.WinConfig
---@field mappings? nvim.Keymap[]
---@field overrides? reform.sig_help.Overrides

---@class reform.sig_help
---@field overrides {set:table<string,fun(reform.Overridable)>,vim:table<string,function|false>}
---@field lsc_on_attach fun(client, buf)|false currently used lsp on_attach override function
---@field defaults {overrides:reform.sig_help.Overrides,config:reform.sig_help.Config}
---@field config reform.sig_help.Config
---@field win {bufnr:integer,id:integer,from_line:integer,to_line:integer,width:integer,cul:integer,cuc:integer,close:fun(self),is_valid:fun(self):boolean,integer[]|nil}
---@field signature {idx:integer,label:string,param_idx:integer,needs_update:fun(self, sig, content_only:boolean):boolean}
---@field toggle fun()
---@field setup fun(config:reform.sig_help.Config)

---@class reform.Config
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field input? reform.Overridable|reform.input.Config reform of vim.ui.input
---@field select? reform.Overridable|reform.util.WinConfig reform of vim.ui.select
---@field link? boolean|reform.link.Config reform of openable links - customizable link detection and actions
---@field toggle? boolean|reform.toggle.Config custom value toggler/changer system - inc/dec/tgl easily
---@field tbl_extras? boolean|reform.tbl_extras.Config multi-table diff, depth-cutoff, printer
---@field sig_help? boolean|reform.sig_help.Config auto-show signature help without blinking

---@class reform
---@field defaults reform.Config
---@field config reform.Config
---@field setup fun(config:reform.Config) setup reform.nvim just the way you like it
