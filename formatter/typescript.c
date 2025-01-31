#include "utils.h"

/**
 * @brief Append lua string to @p fmtPtr.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void add_string(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;
	if (fmt[-1] == '=' || fmt[-1] == ',') *fmt++ = ' ';
	while (*doc == ' ' || *doc == '\n') doc++;
	const in delim = *fmt++ = *doc;
	while (*++doc >= ' ' && *doc != delim) {
		if (*doc == '\\') *fmt++ = *doc++;
		*fmt++ = *doc;
	}
	*fmt++  = *doc;
	*docPtr = doc;
	*fmtPtr = fmt;
}

static const char *types[] = {"string", "any",   "(",  "Function", "number", "integer",
                              "never",  "float", "\"", "'",        "`"};
enum {
	STRING,
	ANY,
	LAMBDA,
	FUNCTION,
	NUMBER,
	INTEGER,
	NEVER,
	FLOAT,
	STR1,
	STR2,
	STR3
};
static const int typeCnt = sizeof(types) / sizeof(const char *);

/**
 * @brief Format type information to a simpler, better readable format.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void type_fmt(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;
	if (fmt[-1] == '=' || fmt[-1] == '>' || fmt[-1] == ':') *fmt++ = ' ';
	do {
		if (*doc == '|') *fmt++ = *doc++;
		while (*doc == ' ') doc++;
		int type = 0, len = 0;
		for (; type < typeCnt; type++, len = 0)
			if ((len = alike(doc, types[type]))) break;
		doc += len;
		switch (type) {
			case STRING:
				if (doc[1] == '=') {
					doc += 2;
					add_string(&doc, &fmt);
					while (*++doc && *doc <= ' ') {}
				} else {
					*fmt++ = '"';
					*fmt++ = '"';
				}
				break;
			case ANY:
				if (*doc != '[') fmt = append(fmt, "any");
				break;
			case LAMBDA: {
				*fmt++         = '(';
				char isWrapped = *doc == '(';
				if (isWrapped) *fmt++ = *doc++;
				if (*doc != ')') {
					char *fmtSave     = fmt;
					const in *docSave = doc;
					char noType       = 0;
					do {
						if (*doc == ',') { // arg separator
							*fmt++ = *doc++;
							*fmt++ = *doc++;
						}
						while (('a' <= *doc && *doc <= 'z') || *doc == '_' || *doc == '?' ||
						       ('A' <= *doc && *doc <= 'Z') || ('0' <= *doc && *doc <= '9'))
							*fmt++ = *doc++; // arg name
						if (*doc == ':') { // arg type
							*fmt++ = *doc++;
							type_fmt(&doc, &fmt);
						} else {
							noType = 1;
							break;
						}
					} while (*doc == ',');
					if (noType) {
						*docPtr = docSave;
						*fmtPtr = fmtSave;
						type_fmt(docPtr, fmtPtr);
						return;
					}
				}
				*fmt++ = ')';
				if (*doc++ == ')' && doc[1] == '=' && doc[2] == '>') { // return value
					if (alike(doc + 4, "void") > 0) {
						fmt = append(fmt, "=>_");
						doc += 8;
					} else {
						fmt = append(fmt, " =>");
						doc += 3;
						type_fmt(&doc, &fmt);
					}
				}
				if (isWrapped) *fmt++ = *doc++;
			} break;
			case FUNCTION: fmt = append(fmt, "()=>_"); break;
			case NUMBER:
			case INTEGER:
				*fmt++ = '0';
				*fmt++ = '.';
				break;
			case NEVER:
				fmt = append(fmt, "NEVER"); // TODO: generics only use these -> catch in code block
				break;
			case FLOAT:
				*fmt++ = '0';
				*fmt++ = '.';
				*fmt++ = '0';
				break;
			case STR1:
			case STR2:
			case STR3:
				doc--;
				add_string(&doc, &fmt);
				doc++;
				break;
			default: { // unknown type / value
				char *fmtTmp = fmt;
				while (('a' <= *doc && *doc <= 'z') || ('A' <= *doc && *doc <= '_') ||
				       ('.' <= *doc && *doc <= '9'))
					*fmt++ = *doc++;
				if (doc[1] == '=') { // skip type info and print value only
					doc += 3;
					fmt = fmtTmp;
				} else if (*doc == '<') { // generics
					*fmt++ = *doc++;
					if (*doc == '{') {
						doc--;
						continue;
					}
					type_fmt(&doc, &fmt);
					while (*doc == ',') {
						*fmt++ = *doc++;
						*fmt++ = ' ';
						type_fmt(&doc, &fmt);
					}
					if (*doc == '>') *fmt++ = *doc++;
				}
			}
		}
		while (*doc == '[' && doc[1] == ']') {
			*fmt++ = '[';
			*fmt++ = ']';
			doc += 2;
		}
		while (*doc == ' ') doc++;
	} while (*doc == '|');
	*docPtr = doc[-1] == ' ' && doc - *docPtr > 2 ? doc - 1 : doc;
	*fmtPtr = fmt;
}

/**
 * @brief Format code (especially typedefs) segment.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @param stop code-ending sequence
 */
static void code_fmt(const in **docPtr, char **fmtPtr, const char *stop) {
	if (!**docPtr) return;
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;
	*fmt++        = *doc;
	int params    = 0; // are we in param/arg section of a function
	// int object    = 0; // are we inside object definition ({})
	while (!alike(++doc, stop) && *doc) {
		switch (*doc) {
			case '"':  // string "..."
			case '\'': // string '...'
			case '`':  // string '...'
				add_string(&doc, &fmt);
				break;
			case '{':
				*fmt++ = '{';
				// if (doc[1] == '\n') object++;
				break;
			case '}':
				// if (fmt[-1] == '\n' && object) object--;
				*fmt++ = '}';
				break;
			case '(':
				// check for "function "
				if (fmt[-1] != ' ') {
					char *fmtTmp = fmt - 1;
					while (*fmtTmp != '\n' && *fmtTmp != ' ') fmtTmp--;
					if ((*fmtTmp == ' ' && alike((in *) fmtTmp - 9, "\nfunction")) || fmt[-1] == '>')
						params = 1;
					else params = -1;
				}
				*fmt++ = '(';
				break;
			case ')':
				*fmt++ = ')';
				if (params) {
					params = 0;
					if (doc[1] == ':') {
						*fmt++ = ':';
						doc += 3;
						type_fmt(&doc, &fmt);
						if (alike(doc, " is ") > 0) {
							fmt = append(fmt, " is ");
							doc += 4;
							type_fmt(&doc, &fmt);
							*fmt++ = '{';
							*fmt++ = '}';
						}
						doc--;
					} else if (doc[2] == '=') break;
					if (*doc == '\n') fmt = append(fmt, " {}");
				}
				break;
			case ':': // type
				*fmt++ = ':';
				if (doc[1] == '\n') {
					doc += 2;
					while (*doc != '|') doc++;
				}
				if (doc[1] == ' ') {
					doc++;
					type_fmt(&doc, &fmt);
					doc--;
				}
				break;
			case '.':
				*fmt++ = '.';
				if (doc[1] != '.' || doc[2] != '.') break;
				*fmt++ = '.';
				*fmt++ = '.';
				doc += 3;
				if (*doc > ' ') type_fmt(&doc, &fmt);
				doc--;
				break;
			case '\\':
				*fmt++ = '\\';
				*fmt++ = *++doc;
				break;
			case '/': // comment
				if (doc[1] == '/')
					while (*doc > '\n') *fmt++ = *doc++;
				else if (doc[1] == '*')
					while (*doc && (*doc != '*' || doc[1] != '/')) *fmt++ = *doc++;
				else *fmt++ = '/';
				break;
			default: *fmt++ = *doc;
		}
	}
	*docPtr = doc;
	*fmtPtr = fmt;
}

/**
 * @brief Format a list item entry as a parameter. Does nothing if item is not a param entry.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void param_fmt(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;

	if (*doc == '|') return;
	else if (*doc == '"') {
		*fmt++ = '*';
		*fmt++ = '"';
		while (*++doc != '"') *fmt++ = *doc;
		*fmt++ = '"';
		*fmt++ = '*';
		if (*++doc == ':') {
			**fmtPtr = '`';
			fmt[-1]  = '`';
			*fmt++   = ':';
			doc++;
		}
	} else {
		if (*doc == '{') doc++;
		if ('A' <= *doc && *doc <= 'Z') return;
		*fmt++ = '`';
		while (('a' <= *doc && *doc <= 'z') || ('A' <= *doc && *doc <= '_') ||
		       ('.' <= *doc && *doc <= '9') || *doc == '\\') {
			if (*doc == '\\') doc++;
			*fmt++ = *doc++;
		}
		*fmt++ = '`';
		int ok = *doc == '}' || *doc == ':';
		if (*doc == '}') doc += doc[2] == ' ' ? 2 : 1;
		else if (ok ? doc[1] != ' ' : *doc != ' ' || doc[1] != '(') return; // it's not a param

		if (*doc == ':') {
			doc++;
			ok = 2;
		}
		if (*doc == ' ' && doc[1] == '(') { // contains type info
			if (ok) {
				*docPtr = doc;
				*fmtPtr = fmt;
			}
			int match = alike(doc + 2, "optional)");
			if (match) {
				if (match < 0) return;
				if (!ok) ok = 1;
				doc += 11;
				*fmt++ = '?';
			} else {
				fmt = append(fmt, " (`");
				doc += 2;
				type_fmt(&doc, &fmt);
				*fmt++ = '`';
				*fmt++ = ')';
				if (*doc++ != ')' || !*doc || (*doc > ' ' && *doc != ':')) {
					if (ok) *(*fmtPtr)++ = ':'; // param without type info
					return;
				}
				if (*doc == ':') {
					doc++;
					ok = 2;
				}
				if ((ok > 1 ? alike(doc, " (optional)") : alike(doc, " optional")) > 0) {
					doc += ok > 1 ? 11 : 10;
					*fmt++ = '?';
				} else if (!ok && *doc != '\n') return;
			}
		} else if (doc[-1] == '}') doc++;

		if (*doc == ':') doc++;
		*fmt++ = ':';
	}

	*docPtr = doc;
	*fmtPtr = fmt;
}

char *typescript_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	const char *fmt0 = fmt; // for checking beginning of output
	fmt              = append(fmt, "```ts\n");
	doc += 14;
	if (*doc == '(') {
		if (doc[1] == 'a') {
			const in *docTmp = doc += 9;
			while (*doc > ' ' && *doc != '(') doc++;
			if (*doc == '(') fmt = append(fmt, "function ");
			doc = docTmp - 1;
		} else if (doc[1] == 'm') {
			fmt = append(fmt, "function"); // (method)
			doc += 8;
		} else {
			*fmt++ = '/';
			*fmt++ = '*';
			while (*++doc != ')') *fmt++ = *doc;
			fmt = append(fmt, "*/");
			doc++;
		}
	} else if (alike(doc, "constructor") > 0) {
		doc += 12;
		fmt = append(fmt, "function ");
		while (*doc != '(') *fmt++ = *doc++;
		fmt = append(fmt, ".constructor");
	}
	code_fmt(&doc, &fmt, "```");
	doc += 3;
	while (*--fmt <= ' ') {}
	fmt = append(fmt + 1, "```\n\n");

	if (doc >= docEnd) return fmt - 1;
	int indent[] = {0, 0, 0}; // 1st lvl, 2nd lvl, 1st lvl set text wrap indent
	char kind    = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '\\':
				*fmt++ = *doc++;
				*fmt++ = *doc;
				break;
			case '`': // code with '```'
				if (doc[1] != '`' || doc[2] != '`') {
					*fmt++ = '`';
					while (*++doc && *doc != '`') *fmt++ = *doc;
					*fmt++ = '`';
					if (*doc != '`') doc--;
				} else {
					fmt = append(fmt, "```");
					if (*(doc += 3) != '\n') // keep original type
						while (*doc > '\n') *fmt++ = *doc++;
					else fmt = append(fmt, "ts");
					if (!*doc) return fmt;
					*fmt++ = *doc++;
					if (alike((in *) fmt - 3, "ts") > 0) code_fmt(&doc, &fmt, "```");
					else
						while (!alike(doc, "```")) *fmt++ = *doc++;
					while (*--fmt <= ' ') {}
					fmt++;
					doc += 3;
					fmt = append(fmt, fmt[-1] == '`' ? "\n```\n" : "```\n\n");
				}
				break;
				break;
			case '<':
				if ('a' <= doc[1] && doc[1] <= 'z') {
					*fmt++ = '*';
					*fmt++ = '<';
					while (*++doc && *doc != '>') *fmt++ = *doc;
					*fmt++ = '>';
					*fmt++ = '*';
				} else *fmt++ = '<';
				break;
			case '|': // vim help-page-style links '|text|'
				if (doc[1] != ' ') {
					char *fmtTmp = fmt;
					*fmt++       = '[';
					while (*++doc != '|' && *doc != ' ' && *doc != '\n') *fmt++ = *doc;
					if (*doc != '|') {
						*fmtTmp = '|';
						doc--;
					} else *fmt++ = ']';
				} else *fmt++ = '|';
				break;
			case '{': {
				const in *docTmp = doc;
				while (*docTmp != ' ' && *docTmp != '}') *fmt++ = *docTmp++;
				if (*docTmp <= ' ' || docTmp - doc == 1) *fmt++ = *docTmp;
				else { // references to params as '{param}'
					fmt[doc - docTmp] = '`';
					*fmt++            = '`';
				}
				doc = docTmp;
			} break;
			case ':': { // highlight other section candidates - capital first letter
				char *fmtTmp = fmt - 1;
				while (fmtTmp > fmt0 && ((*fmtTmp >= 'a' && *fmtTmp <= 'z') || *fmtTmp == '_')) fmtTmp--;
				if (*fmtTmp >= 'A' && *fmtTmp <= 'Z' && // '\n Example:' -> '\n **Example:**'
				    (fmtTmp == fmt0 || fmtTmp[-1] == '\n' || doc[1] == '\n' ||
				     alike((in *) fmtTmp - 2, "\n ") > 0)) {
					for (int m = fmt - --fmtTmp; m; m--) fmtTmp[m + 2] = fmtTmp[m];
					*++fmtTmp = '*';
					*++fmtTmp = '*';
					fmt       = append(fmt + 2, ":**");
					break;
				}
				*fmt++ = ':';
			} break;
			case '@': { // format: '_@param_ — name desc'
				const in *docTmp = doc + 1;
				while (('a' <= *docTmp && *docTmp <= 'z') || *docTmp == '-') docTmp++; // must be a word
				if (*docTmp != '_' || fmt[-1] != '_' || docTmp[1] != ' ') {
					*fmt++ = '@';
					break;
				} else fmt--; // replace the preceding '_'
				doc++;
				resolveKind(&doc, &fmt, &kind);
				if (*doc == '_') doc++;
				docTmp = doc;
				if (kind == 'r' || kind == 'p') fmt = append(fmt, " - ");
				if (kind == 'p' && alike(doc, " — ") > 0) {
					if (*(doc += 5) != '`') *fmt++ = '`';
					while (*doc > ' ') *fmt++ = *doc++;
					if (fmt[-1] != '`') *fmt++ = '`';
					*fmt++ = ':';
					if (alike(doc, " -") > 0) doc += 2;
				} else if (alike(doc, " —") > 0 && (doc += 4)[1] == '-') doc += 3;
				else if (kind == 'E') {               // example
					fmt = append(fmt - 1, "\n```ts\n"); // -1 for the appended '**: '
					while (*doc++ > '\n') {}
					code_fmt(&doc, &fmt, "```");
					while (*--fmt <= ' ') {}
					fmt = append(fmt + 1, "```");
					return fmt;
				} else {
					while (*++doc > ' ') *fmt++ = *doc;
					if (alike(doc, " —") > 0) {
						doc += 4;
						*fmt++ = ':';
					}
				}
				while (*doc == ' ') *fmt++ = *doc++;
				indent[2] += --doc - docTmp - 3;
				if (kind) {
					indent[0] = 1; // we don't know the type so we can't adjust text indent properly
					indent[1] = 0;
				}
			} break;
			case '\n': {
				if (fmt > fmt0 && fmt[-1] == ' ') fmt--;
				if (fmt == fmt0 || fmt[-1] != '\n') *fmt++ = '\n';
				if (alike(++doc, "---\n") > 0) doc += 4;
				int j = 0; // get indentation
				while (doc[j] == ' ') j++;
				if (((doc[j] == '-' || doc[j] == '+' || doc[j] == '*') && doc[j + 1] == ' '
				    ) || // param description
				    alike(doc + j, "• ") > 0) {
					if (!indent[0] || j <= indent[0]) { // 1st lvl indent
						doc += indent[0] = j;
						indent[1] = indent[2] = 0;
					} else if (indent[1] && j > indent[1]) doc += indent[1] - 2; // align to 2nd lvl
					else doc += (indent[1] = j) - 2;                             // 2nd lvl indent
					*fmt++ = ' ';
					while (*doc++ == ' ') *fmt++ = ' ';
					*fmt++ = '-';
					*fmt++ = ' ';
					if (*++doc == 162) {
						doc += 2;
						if (!indent[1]) indent[0] += 2; // `• {}` extra, 2nd lvl is adjusted to that
					}
					param_fmt(&doc, &fmt);                  // format param entry
				} else if (indent[0] && j >= indent[0]) { // wrapped text alignment
					if (indent[1] && j >= indent[1]) doc += indent[1] - 2;
					else if (!indent[1] && indent[2] && j >= indent[2])
						doc += j - indent[2]; // indent with indent[2] spaces
					else doc += indent[0];
					*fmt++ = ' ';
					while (*doc == ' ') *fmt++ = *doc++;
				}
				doc--;
			} break;
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
