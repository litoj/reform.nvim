```lua
(global) behaviour: manipulator.ts.Opts {
    auto_apply_diff_after_generation: boolean = false,
    auto_approve_tool_permissions: boolean|string[] = true,
    buf?: integer,
    depth?: boolean|'leaf'|'leaf-to-root'|'root',
    lang?: boolean|fun(lang: string):boolean|table<string, boolean>,
    lang2?: boolean|(fun(lang: string):boolean|table<string, boolean>),
    range: Range4,
    filter: (fun(state: manipulator.ts.sibling.State, return_reason: 'ancestor_diff'|'lvl_diff'|'max_ancestor'|'max_skip'|test...(+1)):boolean)?,
    enable_fastapply: boolean = false,
    ...(+6)
}
```

---

Specify the behaviour of avante.nvim
1. auto_focus_sidebar              : Whether to automatically focus the sidebar when opening avante.nvim. Default to true.
2. auto_suggestions = false, -- Whether to enable auto suggestions. Default to false.
3. auto_apply_diff_after_generation: Whether to automatically apply diff after LLM response.
                                     This would simulate similar behaviour to cursor. Default to false.
4. auto_set_keymaps                : Whether to automatically set the keymap for the current line. Default to true.
                                     Note that avante will safely set these keymap. See https://github.com/yetone/avante.nvim/wiki#keymaps-and-api-i-guess for more details.
5. auto_set_highlight_group        : Whether to automatically set the highlight group for the current line. Default to true.
6. jump_result_buffer_on_finish = false, -- Whether to automatically jump to the result buffer after generation
7. support_paste_from_clipboard    : Whether to support pasting image from clipboard. This will be determined automatically based whether img-clip is available or not.
8. minimize_diff                   : Whether to remove unchanged lines when applying a code block
9. enable_token_counting           : Whether to enable token counting. Default to true.
