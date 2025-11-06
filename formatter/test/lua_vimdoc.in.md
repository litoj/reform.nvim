```lua
function vim.api.nvim_buf_set_extmark(buffer: integer, ns_id: integer, line: integer, col: integer, opts: vim.api.keyset.set_extmark)
  -> integer
```

---

 Creates or updates an `extmark`.

 By default a new extmark is created when no id is passed in, but it is also
 possible to create a new mark by passing in a previously unused id or move
 an existing mark by passing in its id. The caller must then keep track of
 existing and unused ids itself. (Useful over RPC, to avoid waiting for the
 return value.)

 Using the optional arguments, it is possible to use this to highlight
 a range of text, and also to associate virtual text to the mark.

 If present, the position defined by `end_col` and `end_row` should be after
 the start position in order for the extmark to cover a range.
 An earlier end position is not an error, but then it behaves like an empty
 range (no highlighting).

@*param* `buffer` — Buffer id, or 0 for current buffer

@*param* `ns_id` — Namespace id from `nvim_create_namespace()`

@*param* `line` — Line where to place the mark, 0-based. `api-indexing`

@*param* `col` — Column where to place the mark, 0-based. `api-indexing`

@*param* `opts` — Optional parameters.

 - hl_group: highlight group used for the text range. This and below
     the latter of which can be obtained using `nvim_get_hl_id_by_name()`.

     priority last).
 - hl_eol : when true, for a multiline highlight covering the
            cursorline highlight).
 - virt_text_pos : position of virtual text. Possible values:
   - "eol": right after eol character (default).
 - virt_text_hide : hide the virtual text when the background
                    scrolling with 'nowrap' (or 'smoothscroll').
                    Currently only affects "overlay" virt_text.
 - virt_text_repeat_linebreak: repeat the virtual text on
                               wrapped lines.
 - hl_mode : control how highlights are combined with the
             virt_text highlights, but might affect `hl_group`
   - "blend": blend with background text color.
              Not supported for "inline" virt_text.

 - virt_lines : virtual lines to add next to this mark
     placed below the buffer line containing the mark.

@*return* — Id of the created/updated extmark
