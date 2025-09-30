```cpp
char *typescript_fmt(const unsigned char *doc, char *fmt, int len);
```

typescript/javascript parser

**Parameters**:
 - `doc`: original markdown
 - `fmt`: buffer for formatted documentation
 - `len`: end of `doc`
**Returns**:
 - ptr to current `fmt` position
