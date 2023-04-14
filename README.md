# reform.nvim

Documentation should be informative, concise, and easy to read.
Reform the looks of lsp documentation and possibly a few ui elements with a unifying
documentation parser written in `C`, with primitive fallback parser using `lua` EREs.

## Installation

### [`lazy.nvim`](https://github.com/folke/lazy.nvim)

```lua
return {
  "JosefLitos/reform.nvim",
  event = "VeryLazy",
  build = "make",
  config = true -- automatically call reform.setup(), use [opts] to customize passed table
}
```

### [`packer.nvim`](https://github.com/wbthomason/packer.nvim)

```lua
use {
  "JosefLitos/reform.nvim",
  config = [[require'reform'.setup()]],
  run = "make all",
}
```

## Goals

### Featured

- identical look across different languages (including vim docs in lua)
- as fast as possible - formatted with a single readthrough, written in `C`
- customizable - all functions can be replaced by your own
- any link in markdown format is clickable (see `open_link` bellow)

### Planned

- support more languages
- error handling to avoid entire nvim crash when parsing incorrect markdown
  - probably should fix lua crash with cmp-nvim-lsp-signature-help

## Supported langauges

Bellow mentioned language servers were tested.

- C/Cpp: `clangd`
- Lua: `sumneko_lua`/`lua-language-server` with [`neodev`](https://github.com/folke/neodev.nvim)
- Java: [`nvim-jdtls`](https://github.com/mfussenegger/nvim-jdtls)

<details><summary>

### Screenshots: `with TS` vs. `reform.nvim`

</summary>

- C/Cpp ![C/Cpp](https://user-images.githubusercontent.com/54900518/212124528-7fa9b0b1-9a2e-4b78-be81-e97ace003836.png)
- Lua, including Vim-style documentation ![Lua](https://user-images.githubusercontent.com/54900518/212195668-8463fadf-a0c4-4a4e-b70a-3612a332fead.png)
- Java ![Java](https://user-images.githubusercontent.com/54900518/212200591-deb797c5-c798-4d31-b8c2-3df1a3b9e17b.png)
</details>

## Config

Defaults:

```lua
require'reform'.setup {
  docmd = true|{          -- reform the lsp documentation output
    override = {
      convert = true|fun(), -- reform markdown/docs parser - v.l.u.convert_input_to_markdown_lines
      stylize = true|fun(), -- override with enabled treesitter - vim.lsp.util.stylize_markdown
      cmp_doc = true|fun(), -- replace cmp docs function - require'cmp.entry'.get_documentation
    },
    ft = { -- only boolean values
      c = true, cpp = true, lua = true, java = true
    }
  },
  select = true|fun(),   -- vim.ui.select (used in vim.lsp.buf.code_action)
  open_link = true|{     -- keymappings to open markdown link  (clicked or under cursor)
    {{"", "i"}, "<C-LeftMouse>"},
    {"n", "gl"}
  }
}
```

- **NOTE:** clicking on `file://` links relies on [urlencode](https://github.com/AquilaIrreale/urlencode)
- setup function can be called at any time again to change settings at runtime
