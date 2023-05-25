/**
 * @brief Append `str` to `dst`
 *
 * @param dst destination
 * @param str string to append
 * @return ptr to current `dst` position
 */
char* append(char* dst, const char* str);

/**
 * @brief test if `str` starts with `cmp`
 *
 * @param str tested string
 * @param cmp string to compare against
 * @return length of `cmp` if passed, else `0`
 */
int alike(const char* str, const char* cmp);

/**
 * @brief Parses current doc kind and adds section header when needed.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @param kind kind of previous docs line ('@[]..')
 */
void resolveKind(const char** docPtr, char** fmtPtr, char* kind);
