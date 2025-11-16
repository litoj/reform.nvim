```lua
(upvalue) top_identity: fun(opts: string|manipulator.ts.MethodOpts|manipulator.ts.NodeOpts|manipulator.ts.PresetOpts<manipulator.ts.NodeOpts>, node?: TSNode, ltree: vim.treesitter.LanguageTree, return_par...(too long)...uageTree)?
```

---

Get the furthest ancestor with the same range as current node (and its parent if {parent})

---

```lua
function top_identity(opts: string|manipulator.ts.MethodOpts|manipulator.ts.NodeOpts|manipulator.ts.PresetOpts<manipulator.ts.NodeOpts>, node: TSNode?, ltree: vim.treesitter.LanguageTree, return_parent: boolean?)
  -> TSNode?
  2. (vim.treesitter.LanguageTree)?
```
