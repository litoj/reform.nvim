---@alias reform.util.WinConfig vim.api.keyset.win_config|{winhl:string}
---@alias reform.util.Match {from:integer,to:integer,[integer]:string} all matched groups + bounds of the entire matched text
---@class reform.util.MatchFilter
---@field tolerance? {startPost:integer,startPre:integer,endPost:integer,endPre:integer} startPost=if match starts after cursor, how far after (post-cursor) it can start...
---@field sorting? {order:integer,matcher:integer,offset:integer,length:integer}|fun(ev:reform.util.Event,order:integer,matcher:reform.util.Matcher,match:reform.util.Match):integer|false
---@class reform.util.Event
---@field buf integer
---@field line integer
---@field column integer
---@field filter? reform.util.MatchFilter
---@alias reform.util.Matcher {luapat?:string,vimre?:string,group?:integer,use:fun(match:string,info:reform.util.Match,ev:reform.util.Event):nil|false} returns false for failure, group= lower → higher priority
---@alias reform.util.MatcherMap table<string,reform.util.Matcher>
---@alias reform.util.MatcherRefs (reform.util.Matcher|string)[] matchers or their references by name
---@alias reform.util.MatcherList reform.util.MatcherRefs|fun(event:reform.util.Event):reform.util.MatcherRefs regex & matched text handler pairs or reference to predefined matchers
---@class reform.util
---@field win reform.util.WinConfig default window configuration
---@field filter reform.util.MatchFilter default find_match filter/settings - tolerance + sorting
---@field mk_win fun(buf:integer,opts:reform.util.WinConfig,prompt:string): integer returns window id
---@field exists fun(file:string): boolean checks if given file exists
---@field real_file fun(path:string,bufnr?:integer): string|nil returns real path to the file if file exists relative to cwd or buffer
---@field find_match fun(event:reform.util.Event,matchers:reform.util.MatcherList,knownHandlers:reform.util.MatcherMap,filter:reform.util.MatchFilter):reform.util.Match|false
---@field apply_matcher fun(matcher:reform.util.Matcher,event?:reform.util.Event):reform.util.Match|false
---@field with_mod fun(mod:string,callback:function) run callback when given module is loaded

---@alias reform.Overridable function|boolean defines which function to use - default/plugin default/provided
---@alias nvim.Keymap {[1]:string|string[],[2]:string,[3]?:any} mode, key, opts for gen_mapping
---@alias reform.Config.Mapping.mapping table<string,nvim.Keymap|nvim.Keymap[]>|nvim.Keymap|nvim.Keymap[]
---@class reform.Config.Mapping
---@field mapping? reform.Config.Mapping.mapping mapping for actions - action={modes, binding}
---@class reform.Mapping
---@field gen_mapping fun(bind:nvim.Keymap,action?:string):function generate binding from given keybind config
---@field default_config reform.Config.Mapping
---@field config reform.Config.Mapping

---@alias reform.Config.Override.override table<string,reform.Overridable>
---@class reform.Config.Override
---@field override reform.Config.Override.override picker for custom replacements
---@class reform.Override.override
---@field set table<string,fun(override:function)> setters/activators for custom override
---@field vim table<string,function|false> original functions for restoring default behaviour
---@field reform table<string,function> reform.nvim custom replacements
---@class reform.Override
---@field override reform.Override.override
---@field default_config reform.Config.Override
---@field config reform.Config.Override

---@class reform.docmd.Override map of available lspdocs-related replacements and their settings
---@field convert? reform.Overridable override main lspdocs-to-markdown conversion (vim.lsp.util.convert_input_to_markdown_lines)
---@field stylize? reform.Overridable override docs-display buffer highlighting (vim.lsp.util.stylize_markdown)
---@field convert_sig? reform.Overridable override signature-help docs composition (vim.lsp.util.convert_signature_help_to_markdown_lines)
---@field cmp_doc? reform.Overridable override cmp preview docs parsing ('cmp.entry'.get_documentation)
---@field cmp_sig? reform.Overridable override cmp signature help docs parsing ('cmp_nvim_lsp_signature_help'._docs)

---@class reform.docmd.Config: reform.Config.Override
---@field override? reform.docmd.Override lspdocs-related replacements
---@field ft? table<string,boolean|fun(string,vim.bo.ft):string[]>|true `ft` for which docs should be reformed (if supported)
---@field debug? string|boolean path for writing docs before processing or true for just pringing
---@field labels? table<string,string> map of corrections for markdown filetype labels (' man' → bash)

---@class reform.docmd: reform.Override
---@field default_config reform.docmd.Config
---@field config reform.docmd.Config

---@class reform.ui.Config: reform.Config.Override
---@field win? {input?: reform.util.WinConfig,select?:reform.util.WinConfig}
---@field input_mapping? {hist_prev:string[],hist_next:string[],confirm:string[],cancel:string[]} keymaps for history navigation (insert mode)

---@class reform.ui: reform.Override
---@field default_config reform.ui.Config
---@field config reform.ui.Config

---@class reform.link.Config: reform.Config.Mapping
---@field fallback? 'noop'|'definition'|{copy:boolean,print:boolean,branch:'current'|'default'|fun(ev:reform.util.Event):string}
---       - 'copy','print' the git link generated for current cursor line
---@field filter? reform.util.MatchFilter
---@field matchers? reform.util.MatcherList

---@class reform.link: reform.Mapping
---@field default_config reform.link.Config
---@field config reform.link.Config
---@field matchers reform.util.MatcherMap
---@field get_git_url fun(use_default_branch:boolean,from:integer,to?:integer): string?
---@field handle fun(ev:reform.util.Event) use setup matchers to open link at line:col
---@field key fun() open link under caret
---@field mouse fun() open link under mouse - at click position

---@class reform.toggle.Config: reform.Config.Mapping
---@field mapping? table<string,{[1]:string|string[], [2]:string, [3]:{action?:'tgl'|'inc'|'dec',afterCursor?:boolean,moveCursor?:boolean}}> mapping for actions - mode, keybind, action
---@field matchers? reform.util.MatcherList
---@field filter? reform.util.MatchFilter

---@class reform.toggle.Event: reform.util.Event
---@field setCol? 'start'|'end'|'closer' set cursor to start/end/closer of the match
---@field action 'tgl'|'inc'|'dec' what should the matcher do with the match

---@class reform.toggle: reform.Mapping
---@field default_config reform.toggle.Config
---@field config reform.toggle.Config
---@field replace fun(match:reform.util.Match,ev:reform.util.Event,val:string) replace matched text in buffer with given value
---@field gen_seq_handler fun(seq:string[],matcher?:reform.util.Matcher):reform.util.Matcher create matcher for cycling/toggling between given strings
---@field matchers reform.util.MatcherMap
---@field handle fun(ev:reform.toggle.Event):reform.util.Match? use setup matchers to change value at line:col

---@class reform.tbl_extras.Config: reform.Config.Override
---@field override? {print?: boolean} set custom table printer as extension of the global one
---@field diff? {expand_unique?:any} other tables for the field are `nil` → copy or use custom value
---@field cut_depth? {depth?:integer,cuts?:any} depth to cut off nested tables, replacement for cut-off tables (false to disable)

---@class reform.tbl_extras: reform.Override
---@field default_config reform.tbl_extras.Config
---@field config reform.tbl_extras.Config
---@field tbl_diff fun(opts:{expand_unique?:any},src:table,...:table):table? create diff of table contents compared to src
---@field tbl_short_diff fun(src:table,...:table):table? diff without unique tables expansion
---@field tbl_full_diff fun(src:table,...:table):table? diff with unique tables expansion
---@field tbl_cut_depth fun(tbl:table,opts?:{depth?:integer,cuts?:any}):table cut table at said depth
---@field tbl_print fun(tbl:table) print table with guarantee of visibility to the user

---@class reform.sig_help.Override
---@field lsp_sig? reform.Overridable
---@field lsc_on_attach? reform.Overridable

---@class reform.sig_help.Config: reform.Config.Override,reform.Config.Mapping
---@field override? reform.sig_help.Override
---@field mapppings? {toggle:string[]} keymap for toggling displayed window + auto_show
---@field max_line_offset? integer
---@field max_column_offset? integer
---@field ignore_width_above? number
---@field valid_modes? table<string,boolean>
---@field require_active_param? boolean
---@field auto_show? boolean
---@field win? reform.util.WinConfig

---@class reform.sig_help: reform.Mapping,reform.Override
---@field default_config reform.sig_help.Config
---@field lsc_on_attach fun(client, buf)|false currently used lsp on_attach override function
---@field config reform.sig_help.Config
---@field win {bufnr:integer,id:integer,from_line:integer,to_line:integer,width:integer,cul:integer,cuc:integer,close:fun(self),is_valid:fun(self):boolean,integer[]|nil}
---@field signature {idx:integer,label:string,param_idx:integer,needs_update:fun(self, sig, content_only:boolean):boolean}
---@field toggle fun()

---@class reform.Config
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field ui? boolean|reform.ui.Config reform of vim.ui functions
---@field link? boolean|reform.link.Config reform of openable links - customizable link detection and actions
---@field toggle? boolean|reform.toggle.Config custom value toggler/changer system - inc/dec/tgl easily
---@field tbl_extras? boolean|reform.tbl_extras.Config multi-table diff, depth-cutoff, printer
---@field sig_help? boolean|reform.sig_help.Config auto-show signature help without blinking

---@class reform
---@field default_config reform.Config
---@field config reform.Config
---@field set_override fun(override:reform.Override.override,cfg:reform.Config.Override.override)
---@field set_mapping fun(functions:reform.Mapping,cfg:reform.Config.Mapping.mapping)
---@field setup fun(config:reform.Config) setup reform.nvim just the way you like it
