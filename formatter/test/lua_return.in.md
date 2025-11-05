```lua
function unpack(list: { [1]: <T1>[], [2]: <T2>, [3]: <T3>, [4]: <T4>, [5]: <T5>, [6]: <T6>, [7]: <T7>, [8]: <T8>, [9]: <T9>, [10]: <T10> })
  -> <T1>[]
  2. <T2>
  3. <T3>
  4. <T4>
  5. <T5>
  6. <T6>
  7. <T7>
  8. <T8>
  9. <T9>
 10. <T10>
```

---


Returns the elements from the given `list`. This function is equivalent to
```lua
    return list[i], list[i+1], ···, list[j]
```


[View documents](http://www.lua.org/manual/5.4/manual.html#pdf-unpack)

@*param* `t` — (table) Table

@*return* — : List of values

@*return* — event containing info about the active range

@*return* `is_visual` — true if the range is from visual mode
