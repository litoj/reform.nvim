/**
 * @brief bash/man transform
 *
 * @param doc original markdown
 * @param fmt buffer for formatted documentation
 * @param len end of `doc`
 * @return ptr to current `fmt` position
 */
char *bash_fmt(const char *doc, char *fmt, int len);
