/**
 * @brief lua + vim-style docs parser
 *
 * @param doc original markdown
 * @param fmt buffer for formatted documentation
 * @param len end of `doc`
 * @return ptr to current `fmt` position
 */
char *lua_fmt(const char *doc, char *fmt, int len);
