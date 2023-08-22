# reform.nvim

Documentation should be uniform, concise, and easy to read. Reform the looks of your lsp
documentation with a fast single-pass parser written in `C` and further enhance your experience by
enabling clickable links (fully customizable).

## Goals

### Features

- [x] identical look across different languages (including vim docs in `lua`)
- [x] as fast as possible - formatted on a single pass, written in `C`
- [x] fully customizable - all functions can be replaced by your own
- [x] links should be clickable like in any other IDE (see `open_link` bellow)
- [x] supports `cmp-nvim-lsp-signature-help` - has to replace internal method to inject formatting
- [ ] support `Rust`, `go`

## Installation

### [`lazy.nvim`](https://github.com/folke/lazy.nvim)

```lua
return {
  'JosefLitos/reform.nvim',
  event = 'VeryLazy',
  build = 'make',
  config = true -- automatically call reform.setup(), use [opts] to customize passed table
}
```

### [`packer.nvim`](https://github.com/wbthomason/packer.nvim)

```lua
use {
  'JosefLitos/reform.nvim',
  config = [[require'reform'.setup()]],
  run = 'make',
}
```

## Config

Defaults:

```lua
-- table of config options for `input` and `select`:
local winConfig = {
  title_pos = 'center', -- ↓ title of the prompt replaces `''`
	title = {{'[', 'FloatBorder'}, {'', 'FloatTitle'}, {']', 'FloatBorder'}},
	relative = 'cursor',
	border = 'rounded',
}
require'reform'.setup {
  docmd = true|{          -- reform the lsp documentation output
    override = {
      convert = true|fun(), -- reform markdown/docs parser - v.l.u.convert_input_to_markdown_lines
      stylize = true|fun(), -- override with enabled treesitter - vim.lsp.util.stylize_markdown
      cmp_doc = true|fun(), -- reform cmp docs function - require'cmp.entry'.get_documentation
      cmp_sig = true|fun(), -- reform cmp-nvim-lsp-signature-help formatting function to format MD
    },
    ft = true|{           -- filetypes allowed for parsing (default=all/ ft=true)
      -- lang = name of supported language; boolean/formatter
      lang = true|fun(docs: string, vim.bo.ft): string[]
    },
  },
  input = true|fun()|winConfig,  -- vim.ui.input (used in vim.lsp.buf.rename)
  select = true|fun()|winConfig, -- vim.ui.select (used in vim.lsp.buf.code_action)
  open_link = true|{      -- keymappings to open uri links (clicked or under cursor)
    {{'', 'i'}, '<C-LeftMouse>'},
    {'n', 'gl'},
  },
}
```

- **NOTE:** clicking on `file://` links relies on
  [urlencode](https://github.com/AquilaIrreale/urlencode)
- setup function can be called at any time again to change settings at runtime

## Supported langauges

Language servers bellow were tested.

- Bash: `bashls`
- C/Cpp: `clangd`
- Java: [`nvim-jdtls`](https://github.com/mfussenegger/nvim-jdtls)
- Lua: `sumneko_lua`/`lua-language-server` with [`neodev`](https://github.com/folke/neodev.nvim)
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
