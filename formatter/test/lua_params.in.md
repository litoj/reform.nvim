```lua
function pcall(f: fun(...any):...unknown, arg1?: any, ...any)
  -> success: string
  2. result: any
  3. ...any
```

---


Calls the function `f` with the given arguments in *protected mode*. This means that any error inside `f` is not propagated; instead, `pcall` catches the error and returns a status code. Its first result is the status code (a boolean), which is true if the call succeeds without errors. In such case, `pcall` also returns all results from the call, after this first result. In case of any error, `pcall` returns `false` plus the error object.


[View documents](http://www.lua.org/manual/5.4/manual.html#pdf-pcall)
