```lua
function vim.treesitter.LanguageTree:node_for_range(range = Range4, opts = nil|vim.treesitter.LanguageTree.tree_for_range.Opts) end
  -> vim.api.Test|nil, node_info = { node = TSNode, lang = ''|nil, path = { node = TSNode, lang = ''}}|nil, ''[]
```

Gets the set of included regions managed by this LanguageTree. This can be different from the
regions set by injection query, because a partial [LanguageTree:parse()] drops the regions
outside the requested range.
Each list represents a range in the form of
{ `start_row`, `start_col`, `start_bytes`, `end_row`, `end_col`, `end_bytes` }.
