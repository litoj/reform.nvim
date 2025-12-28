```lua
(field) manipulator.Batch|manipulator.CallPath.TS|manipulator.Region.pick: function|manipulator.CallPath.TS|{ [string]: fun(...any):manipulator.CallPath.TS|{ [string]: manipulator.CallPath.TS }|manipulator.CallPath.TS|{ [string]: manipulator.CallPath.TS } } {
    action_opts: function,
    add_to_qf: function,
    as_op: function,
    as_qf_item: function,
    backup?: any,
    buf: integer,
    child: function,
    clone: function,
    collect: function,
    contains: function,
    ...(+42)
}
```

---

 Pick from the items. Must be in a coroutine or use a callback.

@*param* `opts` — can include all `fzf-lua.config.Base` opts

---

```lua
(method) manipulator.Batch:pick(opts?: manipulator.Batch.pick.Opts)
  -> (manipulator.Batch|manipulator.Region)?
```

---

```lua
(method) ()
  -> manipulator.CallPath.TS|{ [string]: manipulator.CallPath.TS }
```
