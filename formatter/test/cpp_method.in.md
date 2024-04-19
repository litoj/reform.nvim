### function `cpp_fmt`

---
→ `char *`
Parameters:
- `const in * doc (aka const unsigned char *)`
- `char * fmt`
- `int len`

@brief C and CPP doxygen parser
@param doc original markdown
@param fmt buffer for formatted documentation
@param len end of `doc`
@return ptr to current `fmt` position

---
```cpp
char *cpp_fmt(const in *doc, char *fmt, int len)
```
