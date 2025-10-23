#ifndef utils
#define utils 1
typedef unsigned char in;
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
int alike(const in *str, const char *cmp);

/**
 * @brief Parses current doc kind and adds section header when needed.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @param kind kind of previous docs line ('@[]..')
 */
void resolveKind(const in **docPtr, char **fmtPtr, char *kind);

#define empty(ch) ((ch) <= ' ' && (ch))
#define isCONST(ch) (('A' <= (ch) && (ch) <= 'Z') || ('0' <= (ch) && (ch) <= '9') || (ch) == '_')
/**
 * @brief matches object path, lua type path and file path (`-`, `.`, `/`, `_`)
 */
#define isPath(ch)                                                                                 \
	(('a' <= (ch) && (ch) <= 'z') || (ch) == '_' || ('A' <= (ch) && (ch) <= 'Z') ||                  \
	 ('-' <= (ch) && (ch) <= '9'))
#define isVar(ch)                                                                                  \
	(('a' <= (ch) && (ch) <= 'z') || (ch) == '_' || ('A' <= (ch) && (ch) <= 'Z') ||                  \
	 ('0' <= (ch) && (ch) <= '9'))
#endif
