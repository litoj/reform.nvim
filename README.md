# reform.nvim

Documentation should be uniform, concise, and easy to read. Reform the looks of your lsp
documentation with a single-pass formatter written in `C` and further enhance your experience by
enabling clickable links (all fully customizable).

## Goals

### Features

- [x] identical look across different languages (including vim docs in `lua`)
- [x] speed is key - formatted on a single pass, written in `C`
- [x] fully customizable - all functions can be replaced by your own
- [x] `vim`.`ui`.`input`/`select` popups float at the cursor (instead of cmdline)
- [x] clickable links like any other IDE (see `open_link`)
- [x] `cmp-nvim-lsp-signature-help` support - has to replace internal method to inject formatting
- [x] manpager with automatic formatting using docfmt(bash) (see `man`)
  - [ ] `man(5)` references should be clickable + highlighted as links (add `:Man` support in link)
  - [ ] improve speed by using a modified bash parser
- [ ] support `Rust`, `go`

## Installation with [`lazy.nvim`](https://github.com/folke/lazy.nvim)

```lua
return {
  'JosefLitos/reform.nvim',
  event = 'LspAttach',
  build = 'make',
  opts = {} -- put settings here
}
```

## Config

Defaults:

```lua
-- table of config options for `input` and `select`:
local winConfig = {
  title_pos = 'center', --        ↓ title of the prompt replaces `''`
	title = {{'[', 'FloatBorder'}, {'', 'FloatTitle'}, {']', 'FloatBorder'}},
	relative = 'cursor',
	border = 'rounded',
}
require'reform'.setup {
  docmd = true|{ -- reform the lsp documentation output
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
  open_link = true|{ -- keymappings to open uri links (clicked or under cursor)
    {{'', 'i'}, '<C-LeftMouse>'},
    {'n', 'gl'},
  },
  man = false -- custom manpage formatting (using docfmt(bash))
}
```

- setup function can be called at any time again to change settings at runtime

## Supported langauges

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
