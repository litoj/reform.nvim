```lua
function top_identity(opts = ''|manipulator.ts.MethodOpts|manipulator.ts.NodeOpts|manipulator.ts.PresetOpts<manipulator.ts.NodeOpts>, node = TSNode|nil, ltree = vim.treesitter.LanguageTree, return_parent = bool|nil) end
  =-> TSNode|nil, (vim.treesitter.LanguageTree)|nil
```

Get the furthest ancestor with the same range as current node (and its parent if `parent`)
