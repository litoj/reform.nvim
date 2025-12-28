```lua
top_identity = fun(opts = ''|fzf-lua.test|manipulator.ts.PresetOpts<manipulator.ts.NodeOpts>, node = nil|TSNode, ltree = vim.treesitter.LanguageTree, return_par...)|nil
```

Get the furthest ancestor with the same range as current node (and its parent if `parent`)

```lua
function top_identity(opts = ''|fzf-lua.test|Opts<ts.NodeOpts, <B>, <C = {x = 0.}>>, node = TSNode|nil, ltree = vim.treesitter.LanguageTree, return_parent = bool|nil) end
  =-> TSNode|nil, (vim.treesitter.LanguageTree)|nil
```
