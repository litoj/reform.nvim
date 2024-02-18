# reform.nvim

Documentation should be uniform, concise, and easy to read. This plugin aims to reform neovim to
feel more like an IDE with a few QoL improvements (all fully customizable).

### Features

- fully customizable - all options expandable with your own functions for dynamic decissions
- `docmd`: reformat lsp docs for a **consistent docs structure**
  - identical look across supported languages (including vim docs in `lua`)
  - per-ft configurable
  - opt-in support for all places using lsp docs
    - signature help with param highlighting included (`cmp-nvim-lsp-signature-help`) and builtin
  - speed is key - written in `C` with simplified utf8 support
  - [ ] support more languages - `Rust`, `go`…
- `open_link`: clickable links like any other IDE
  - keybind creation builtin, but fully optional
  - modular handlers by regex → anything can be matched/considered a link
    - handler for stacktrace filepaths with cursor position supporting terminal line-wrapping
    - uri handlers, nvim plugin link handler… (see default config)
- `vim`.`ui`.`input`/`select` as popups floating at the cursor (instead of cmdline)

### Installation with [`lazy.nvim`](https://github.com/folke/lazy.nvim)

```lua
return {
  'JosefLitos/reform.nvim',
  event = 'LspAttach',
  opts = {} -- put settings here
}
```

### Config defaults

```lua
-- table of config options for `input` and `select`:
local winConfig = {
  title_pos = 'center', --        ↓ title of the prompt replaces `''`
  title = {{'[', 'FloatBorder'}, {'', 'FloatTitle'}, {']', 'FloatBorder'}},
  relative = 'cursor',
  border = 'rounded',
}
require'reform'.setup {
  docmd = true|{ -- reformat language-server's docs markdown to have a consistent structure
    override = {
      convert = true|fun(), -- main lspdocs-to-markdown conversion
      stylize = true|fun(), -- docs-display buffer highlighting
      convert_sig = true|fun(), -- signature-help docs composition
      cmp_doc = true|fun(), -- cmp preview docs parsing
      cmp_sig = true|fun(), -- cmp signature help docs parsing
    },
    ft = true|{ -- filetypes allowed for parsing (default=all/ ft=true)
      -- lang = name of supported language; boolean/formatter
      lang = true|fun(docs: string, vim.bo.ft): string[]
    },
    debug = false|'', -- filename for saving received docs before parsing (for crash debugging)
  },
  input = true|fun()|{ -- vim.ui.input (used in vim.lsp.buf.rename)
    window = { height = 1, row = -3}+winConfig,
    keymaps = { -- keybinds are replaced per action -> cancel={'<C-q>'} removes <Esc>
      cancel = { '<Esc>', '<C-q>' },
      confirm = { '<CR>' },
      histPrev = { '<Up>', '<M-k>' },
      histNext = { '<Down>', '<M-j>' },
    },
  },
  select = true|fun()|{ -- vim.ui.select (used in vim.lsp.buf.code_action)
    col = -2, row = 1, winhl = 'Id:Repeat,VarDelim:Delimiter'
  }+winConfig,
  open_link = true|{ -- under-cursor-regex matcher with configurable actions
    mappings = { -- keymappings to open uri links (clicked or under cursor)
      {{'', 'i'}, '<C-LeftMouse>'}, -- maps to open_link.mouse()
      {'n', 'gL'},                  -- maps to open_link.key()
    },
    handlers = { -- return value as an exit value → try other handlers if matched handler failed
			-- Event fields: buf, line, column, mouse (if generated by a mouse click)
      {pattern = 'reg(match)exp', use = fun(match:string, reform.open_link.Event): 1?}
      'markdown_url',         -- [name](http://url)
      'any_url',              -- http://url
      'markdown_file_uri',    -- [name](file:///path/to/file)
      'markdown_file_path',   -- [name](/file/path)
      'reform_vimdoc_ref',    -- [VimHelpLink]
      'vimdoc_ref',           -- |VimHelpLink|
      'stacktrace_file_path', -- ~/multiline/path/to/file:line:column
      'nvim_plugin',          -- 'JosefLitos/reform.nvim'
    },
    fallback = 'definition' -- action on no match - invalid value / 'noop' means no action
      -- git link generation: {copy: boolean, print: boolean, branch:'default'|'current'|fun(ev)}
      -- generates links to referenced line in the 'default'/'current'/provided branch
  },
  man = false -- custom manpage formatting (using formatter(bash))
}
```

- setup function can be called at any time again to change the settings at runtime

## Formatting-supported langauges

Language servers bellow were tested.

- Bash: `bashls`
- C/Cpp: `clangd`
- Java: [`nvim-jdtls`](https://github.com/mfussenegger/nvim-jdtls)
- Lua: `lua-language-server` with [`neodev`](https://github.com/folke/neodev.nvim)
- Typescript/Javascript: `typescript-launguage-server`/`typescript-tools.nvim`

<details><summary>

### Screenshots: `with TS` vs. `reform.nvim`

</summary>

- `bashls`
  ![Bash/sh](https://github.com/JosefLitos/reform.nvim/assets/54900518/8a66cac0-52a9-4672-adae-9c44bc3cf3c4)
- `clangd`
  ![C/C++](https://github.com/JosefLitos/reform.nvim/assets/54900518/ccbac42a-f2a6-4ffd-8abd-c3e3d2d81c78)
- `typescript-language-server`
  ![Javascript/Typescript](https://github.com/JosefLitos/reform.nvim/assets/54900518/a0e954a4-429f-4d9a-a460-5525678a8c0c)
- `jdtls`
  ![Java](https://user-images.githubusercontent.com/54900518/212200591-deb797c5-c798-4d31-b8c2-3df1a3b9e17b.png)
- `luals`, including Vim-style documentation
  ![Lua](https://user-images.githubusercontent.com/54900518/212195668-8463fadf-a0c4-4a4e-b70a-3612a332fead.png)

</details>
