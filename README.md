# reform.nvim

Reform the looks of lsp documentation and possibly a few ui elements.
Documentation parser written in `C`, with `lua` regex modifications as fallback.

## Installing with [`packer`](https://github.com/wbthomason/packer.nvim)

```lua
use {
  "JosefLitos/reform.nvim",
  config = [[require'reform'.setup()]],
  run = "make docfmt",
}
```

## Config

Defaults:
```lua
require'reform'.setup { -- values are `boolean` or replacement `function`
  input = true|fun(),  -- vim.ui.input (used in vim.lsp.buf.rename)
  select = true|fun(), -- vim.ui.select (used in vim.lsp.buf.code_action)
  docmd = true|{       -- reform the lsp documentation output
    override = {
      convert = true|fun(), -- reform markdown/docs parser - v.l.u.convert_input_to_markdown_lines
      stylize = true|fun(), -- override with enabled treesitter - vim.lsp.util.stylize_markdown
      cmp_doc = true|fun(), -- replace cmp docs function - require'cmp.entry'.get_documentation
    },
    ft = { -- only boolean values
      c = true, cpp = true, lua = true
    }
  }
}
```
