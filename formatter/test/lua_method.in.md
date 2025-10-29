```lua
(method) vim.treesitter.LanguageTree:node_for_range(range: Range4, opts?: vim.treesitter.LanguageTree.tree_for_range.Opts)
  -> vim.api.Test?
  2. node_info: { node: TSNode, lang: string?, path: { node: TSNode, lang: string } }?
  3. string[]
```

---

Gets the set of included regions managed by this LanguageTree. This can be different from the
regions set by injection query, because a partial |LanguageTree:parse()| drops the regions
outside the requested range.
Each list represents a range in the form of
{ {start_row}, {start_col}, {start_bytes}, {end_row}, {end_col}, {end_bytes} }.
