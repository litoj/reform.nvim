```lua
local mcp: manipulator.call_path {
    exec: function,
    exec_on_call: boolean|integer,
    fn: function,
    new: function,
    region: fun(opts: manipulator.Region.current.Opts):manipulator.call_path.region|{ [string]: fun(...any):manipulator.call_path.region|{ [string]: manipulator.call_path.region }|manipulator....(too long)...region } },
    ts: fun(opts: manipulator.ts.current.Opts):manipulator.call_path.ts|{ [string]: fun(...any):manipulator.call_path.ts|{ [string]: manipulator.call_path.ts }|manipulator.call_path.ts|{ [...(too long)...ath.ts } },
    __call: function,
    __index: function,
}
```

---

Field `.exec_on_call`:
 - `number` in ms until actual execution - updates itself,
 - `false` to not execute until manual call of `:exec()`,
 - `true` to `:exec()` calls immediately - returns a new wrapper.
**NOTE:** for path assembly and call:
 - for keymappings reference the `.fn` field
 - for reusable paths use `x:new()` to clone the path built until that point
 - for direct evaluation call twice (`x:y(opts?)()` or `x.y()()`)
   this will evaluate the whole path without saving the new callee to the old callpath,
   or call `:exec()` or `.fn()` manually (saves the last callee to the original callpath)
