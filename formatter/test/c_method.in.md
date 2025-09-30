### function `typescript_fmt`

---
→ `char *`
Parameters:
- `const unsigned char * doc`
- `char * fmt`
- `int len`

@brief typescript/javascript parser
@param doc original markdown
@param fmt buffer for formatted documentation
@param len end of `doc`
@return ptr to current `fmt` position

---
```objective-cpp
char *typescript_fmt(const unsigned char *doc, char *fmt, int len)
```
