```lua
function unpack(list = { [1] = <T1>[], [2] = <T2>, [3] = <T3>, [4] = <T4>, [5] = <T5>, [6] = <T6>, [7] = <T7>, [8] = <T8>, [9] = <T9>, [10] = <T10>}) end
  =-> <T1>[], <T2>, <T3>, <T4>, <T5>, <T6>, <T7>, <T8>, <T9>, <T10>
```

Returns the elements from the given `list`. This function is equivalent to
```lua
    return list[i], list[i+1], ···, list[j]
```

[View documents](http://www.lua.org/manual/5.4/manual.html#pdf-unpack)

**Parameters**:
 - `t`: (table) Table
**Returns**:
 - List of values
 - event containing info about the active range
 - `is_visual`: true if the range is from visual mode
