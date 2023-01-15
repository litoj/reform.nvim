/**
 * @brief C and CPP doxygen parser
 *
 * @param doc original markdown
 * @param fmt buffer for formatted documentation
 * @param len end of `doc`
 * @return ptr to current `fmt` position
 */
char *cpp_fmt(const char *doc, char *fmt, int len);
