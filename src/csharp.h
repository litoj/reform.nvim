/**
 * @brief C# parser
 *
 * @param doc original markdown
 * @param fmt buffer for formatted documentation
 * @param len end of `doc`
 * @return ptr to current `fmt` position
 */
char *csharp_fmt(const unsigned char *doc, char *fmt, int len);
