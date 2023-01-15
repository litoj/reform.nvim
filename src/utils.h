/**
 * @brief Append `str` to `dst`
 *
 * @param dst destination
 * @param str string to append
 * @return ptr to current `dst` position
 */
char *append(char *dst, const char *str);

/**
 * @brief test if `str` starts with `cmp`
 *
 * @param str tested string
 * @param cmp string to compare against
 * @return length of `cmp` if passed, else `0`
 */
int alike(const char *str, const char *cmp);

/**
 * @brief Parses current doc kind and adds section header when needed.
 *
 * @param doc source docs with original text
 * @param fmt buffer for formatted docs
 * @param docPos ptr to current `doc` index
 * @param fmtPos ptr to current `fmt` index
 * @param kind kind of previous docs line ('@[]..')
 * @return ptr to current `fmt` position
 */
char *resolveKind(const char *doc, char *fmt, int *docPos, char *kind);
