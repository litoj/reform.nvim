```lua
manipulator.Batch|manipulator.CallPath.TS|manipulator.Region.pick = fun()|manipulator.CallPath.TS|{ [''] = fun(...?):manipulator.CallPath.TS|{ [''] = manipulator.CallPath.TS }|manipulator.CallPath.TS|{ [''] = manipulator.CallPath.TS } } = {
    action_opts = fun(),
    add_to_qf = fun(),
    as_op = fun(),
    as_qf_item = fun(),
    backup = nil|?,
    buf = 0.,
    child = fun(),
    clone = fun(),
    collect = fun(),
    contains = fun(),
    ...
}
```

 Pick from the items. Must be in a coroutine or use a callback.

**Parameters**:
 - `opts`: can include all `fzf-lua.config.Base` opts

```lua
function manipulator.Batch:pick(opts = nil|manipulator.Batch.pick.Opts) end
  =-> (manipulator.Batch|manipulator.Region)|nil
```

```lua
function _() end
  =-> manipulator.CallPath.TS|{ [''] = manipulator.CallPath.TS }
```
