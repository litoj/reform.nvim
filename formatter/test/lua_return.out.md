```lua
function unpack(list = { [1] = test<T1>[], [2] = test.path<T2>, [3] = <T3>, [10] = <T10> }) end
  =-> test<T1>[], test.path<T2>, <T3>, <T10>
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
