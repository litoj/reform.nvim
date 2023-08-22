---@alias reform.Overridable function|boolean defines which function to use - default/plugin default/provided
---@alias nvim.hiGroup string vim highlight group name
---@alias nvim.Keymap {[1]:string|table<string,string>, [2]: string} mode and key for vim.keymap.set
---@alias reform.Keymappings table<integer,nvim.Keymap>

---@class reform.docmd.Config
---@field public override {convert: reform.Overridable, stylize: reform.Overridable, cmp_doc: reform.Overridable, cmp_sig: reform.Overridable}
---@field public ft table<string,boolean|fun(string,vim.bo.ft):string[]>|true docs for which `ft` should be reformed (if supported)

---@class reform.docmd.Defaults: reform.docmd.Config
---@field public config reform.docmd.Config

---@class reform.winConfig config options for popup windows created by `input`/`select`
---@field public title_pos "left"|"center"|"right"|nil position of window title
---@field public title table<integer,{[1]:string,[2]:nvim.hiGroup}>|nil prefix
---@field public relative "editor"|"win"|"cursor"|"mouse"|nil
---@field public win integer|nil
---@field public anchor "NW"|"NE"|"SW"|"SE"|nil
---@field public height integer|nil
---@field public row integer|nil
---@field public col integer|nil
---@field public border "none"|"single"|"double"|"rounded"|"solid"|"shadow"|table<string>|nil

---@class reform.Config config options for reform.nvim
---@field public docmd boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field public input reform.Overridable|reform.winConfig reform of vim.ui.input
---@field public select reform.Overridable|reform.winConfig reform of vim.ui.select
---@field public open_link boolean|reform.Keymappings reform of openable links - set your custom shortcuts
