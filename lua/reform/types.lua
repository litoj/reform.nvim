---@alias reform.Overridable function|boolean defines which function to use - default/plugin default/provided
---@alias nvim.hiGroup string vim highlight group name
---@alias nvim.Keymap {[1]:string|table<string,string>, [2]: string} mode and key for vim.keymap.set
---@alias reform.Keymappings table<integer,nvim.Keymap>
---@alias reform.WinConfig vim.api.keyset.float_config|{winhl:table<string,string>}

---@class reform.docmd.Config
---@field override? {convert: reform.Overridable, stylize: reform.Overridable, cmp_doc: reform.Overridable, cmp_sig: reform.Overridable}
---@field ft? table<string,boolean|fun(string,vim.bo.ft):string[]>|true docs for which `ft` should be reformed (if supported)

---@class reform.docmd.Defaults: reform.docmd.Config
---@field config reform.docmd.Config

---@class reform.input.Config
---@field window? reform.WinConfig
---@field keymaps? {histPrev: table<string>, histNext: table<string>}

---@class reform.Config config options for reform.nvim
---@field docmd? boolean|reform.docmd.Config reform of all markdown documentation/signature...
---@field input? reform.Overridable|reform.WinConfig reform of vim.ui.input
---@field select? reform.Overridable|reform.input.Config reform of vim.ui.select
---@field open_link? boolean|reform.Keymappings reform of openable links - set your custom shortcuts
