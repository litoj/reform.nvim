#include "utils.h"

/**
 * @brief Append lua string to @p fmtPtr.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void add_string(const char** docPtr, char** fmtPtr) {
	const char* doc = *docPtr;
	char* fmt       = *fmtPtr;
	if (fmt[-1] == '=' || fmt[-1] == ',') *fmt++ = ' ';
	while (*doc == ' ' || *doc == '\n') doc++;
	*fmt++ = *doc;
	switch (*doc) {
		case '"': // string "..."
			while (*++doc >= ' ' && *doc != '"') {
				if (*doc == '\\') *fmt++ = *doc++;
				*fmt++ = *doc;
			}
			break;
		case '\'': // string '...'
			while (*++doc >= ' ' && *doc != '\'') {
				if (*doc == '\\') *fmt++ = *doc++;
				*fmt++ = *doc;
			}
			break;
		case '[': // string [[...]]
			if (doc[1] != '[') return;
			while (*++doc && (*doc != ']' || doc[1] != ']')) {
				if (*doc == '\\') *fmt++ = *doc++;
				*fmt++ = *doc;
			}
			*fmt++ = *doc++;
			break;
		default:
			return;
	}
	*fmt++  = *doc;
	*docPtr = doc;
	*fmtPtr = fmt;
}

static const char* types[] = {"float", "number", "integer", "string", "any", "unknown", "table",
  "[", "boolean", "function", "fun(", "<", "dictionary"};

/**
 * @brief Format type information to a simpler, better readable format.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void lua_type_fmt(const char** docPtr, char** fmtPtr) {
	const char* doc = *docPtr;
	char* fmt       = *fmtPtr;
	if (fmt[-1] == '=' || fmt[-1] == ',' || fmt[-1] == ':') *fmt++ = ' ';
	do {
		if (*doc == '|') *fmt++ = *doc++;
		while (*doc <= ' ') doc++;
		char backTicks = *doc == '`';
		if (backTicks) doc++;
		int type = 0, len = 0;
		for (; type < 13; type++, len = 0)
			if ((len = alike(doc, types[type]))) break;
		doc += len;
		if (*doc == '[') *fmt++ = '{';
		switch (type) {
			case 0: // float
				*fmt++ = '0';
			case 1: // number
				*fmt++ = '.';
				*fmt++ = '0';
				break;
			case 2: // integer
				*fmt++ = '0';
				*fmt++ = '.';
				break;
			case 3: // string
				if (doc[1] == '=') {
					doc += 2;
					add_string(&doc, &fmt);
					while (*++doc && *doc <= ' ') {}
				} else {
					*fmt++ = '"';
					*fmt++ = '"';
				}
				break;
			case 4: // any
			case 5: // unknown
				*fmt++ = '?';
				break;
			case 6: // table
				if (*doc == '<') {
					*fmt++ = '{';
					doc++;
					if (*doc != '>') {
						for (type = 0; type < 4; type++, len = 0)
							if ((len = alike(doc, types[type]))) break;
						if (type < 4 && doc[len] != '|') {
							doc += len;
							if (type == 3) fmt = append(fmt, "val ="); // string index -> it's a map/dictionary
						} else {
							*fmt++ = '['; // indexing with a more compicated type
							lua_type_fmt(&doc, &fmt);
							fmt = append(fmt, "] =");
						}
						if (*doc == '>') { // only indexes specified, no field type
							if (fmt[-1] == '=') *fmt++ = ' ';
							*fmt++ = '?';
						} else { // resolve type of field
							doc++;
							lua_type_fmt(&doc, &fmt);
						}
					}
					while (*doc++ != '>') {} // strip all extra useless table data
					*fmt++ = '}';
				} else if (*doc == ' ' && doc[1] == '{') doc++;
				else { // unspecified table - 'table'
					*fmt++ = '{';
					*fmt++ = '}';
				}
				break;
			case 7: // '[' - manually denoted array
				*fmt++ = '{';
				if (*doc != ']') // table/array has data inside
					do {
						if (*doc == ',') *fmt++ = *doc++;
						lua_type_fmt(&doc, &fmt);
					} while (*doc == ',');
				*fmt++ = '}';
				doc++;
				break;
			case 8: // boolean
				fmt = append(fmt, "true|false");
				break;
			case 9: // function
				fmt = append(fmt, "fun()");
				break;
			case 10: // fun( - function with specified args
				fmt = append(fmt, "fun(");
				if (*doc != ')') do {
						if (*doc == ',') { // arg separator
							*fmt++ = *doc++;
							*fmt++ = *doc++;
						}
						while (('a' <= *doc && *doc <= 'z') || *doc == '_' || *doc == '?' ||
						  ('A' <= *doc && *doc <= 'Z') || ('0' <= *doc && *doc <= '9'))
							*fmt++ = *doc++; // arg name
						if (*doc == ':') { // arg type
							*fmt++ = *doc++;
							lua_type_fmt(&doc, &fmt);
						}
					} while (*doc == ',');
				*fmt++ = ')';
				if (*doc++ == ')' && *doc == ':') { // return value
					doc++;
					fmt = append(fmt, " -> ");
					lua_type_fmt(&doc, &fmt);
				}
				break;
			case 11: // generics
				*fmt++ = '<';
				while (('A' <= *doc && *doc <= 'Z') || ('0' <= *doc && *doc <= '9')) *fmt++ = *doc++;
				if (*doc == ':') {
					*fmt++ = *doc++;
					lua_type_fmt(&doc, &fmt);
				}
				*fmt++ = *doc++;
				if (*doc == '[' && doc[1] == ']') {
					*fmt++ = '[';
					*fmt++ = ']';
					doc += 2;
				}
				break;
			case 12: // dictionary
				fmt = append(fmt, "{val = \"\"}");
				break;
			default: { // unknown type / value (nil, ...)
				char* fmtTmp = fmt;
				while (('a' <= *doc && *doc <= 'z') || *doc == '_' || *doc == '.' ||
				  ('A' <= *doc && *doc <= ']') || ('0' <= *doc && *doc <= '9'))
					*fmt++ = *doc++;
				if (doc[1] == '=') { // skip type info and print value only
					doc += 3;
					fmt = fmtTmp;
				}
			}
		}
		if (*doc == '[' && doc[1] == ']') {
			*fmt++ = '}';
			doc += 2;
		}
		if (backTicks && *doc == '`') {
			doc++;
			while (*doc == ' ' || *doc == '\n') doc++;
		}
	} while (*doc == '|');

	*docPtr = doc;
	*fmtPtr = fmt;
}

/**
 * @brief Format lua code from sumneko_lua markdown docs.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @param stop code-ending sequence
 */
static void lua_code_fmt(const char** docPtr, char** fmtPtr, const char* stop) {
	if (!**docPtr) return;
	const char* doc = *docPtr;
	char* fmt       = *fmtPtr;
	*fmt++          = *doc;
	int params      = 0; // whether we are in param/arg section of a function definition
	while (!alike(++doc, stop) && *doc) {
		switch (*doc) {
			case '[': // string [[...]]
				if (doc[1] != '[') {
					*fmt++ = '[';
					break;
				}
			case '"':  // string "..."
			case '\'': // string '...'
				add_string(&doc, &fmt);
				break;
			case '(':
				// check for "function "
				if (fmt[-1] != ' ' || (fmt[-2] == 'n' && fmt[-9] == 'f')) {
					char* fmtTmp = fmt - 1;
					while (*fmtTmp != '\n' && *fmtTmp != ' ') fmtTmp--;
					if (*fmtTmp == ' ' && alike(fmtTmp - 9, "\nfunction")) {
						if (fmt[-1] == ' ') *fmt++ = '_'; // give a name to unnamed function
						params = 1;
					}
				}
				*fmt++ = '(';
				break;
			case ')':
				*fmt++ = ')';
				if (params) {
					params = 0;
					if (alike(doc + 1, " end") <= 0) fmt = append(fmt, " end");
				}
				break;
			case '\n':
				*fmt++ = '\n';
				{ // numbered return type - multiple return options - `  2. retVal`
					if (doc[1] != ' ' || doc[2] != ' ') break;
					doc += 2;
					*fmt++             = ' ';
					*fmt++             = ' ';
					const char* docTmp = doc + 1;
					while ('0' <= *docTmp && *docTmp <= '9') docTmp++;
					if (*docTmp != '.') break;
					doc    = docTmp;
					*fmt++ = '-';
				}
			case '>': // return type
				*fmt++ = '>';
				if (fmt[-2] != '-' || doc[1] != ' ') break;
				*fmt++ = *++doc;
				lua_type_fmt(&doc, &fmt);
				doc--;
				break;
			case ':': // type
				if (doc[1] == '\n') {
					doc += 2;
					while (*doc != '|') doc++;
				}
				if (doc[1] == ' ') {
					if (params) *fmt++ = ':';
					else {
						*fmt++ = ' ';
						*fmt++ = '=';
					}
					doc++;
					lua_type_fmt(&doc, &fmt);
					doc--;
				} else *fmt++ = *doc;
				break;
			case '.':
				*fmt++ = '.';
				if (doc[1] != '.' || doc[2] != '.') break;
				*fmt++ = '.';
				*fmt++ = '.';
				doc += 3;
				if (*doc > ' ') lua_type_fmt(&doc, &fmt);
				doc--;
				break;
			case '\\':
				*fmt++ = '\\';
				*fmt++ = *++doc;
				break;
			case '-':
				if (doc[1] == '-') { // lua comment
					if (doc[2] == '[' && doc[3] == '[') add_string(&doc, &fmt);
					else
						while (*doc > '\n') *fmt++ = *doc++;
					if (*doc && doc[1] != '\n') *fmt++ = '\n';
					break;
				}
			default:
				*fmt++ = *doc;
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
static void lua_param_fmt(const char** docPtr, char** fmtPtr) {
	const char* doc = *docPtr;
	char* fmt       = *fmtPtr;

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
		while (('a' <= *doc && *doc <= 'z') || *doc == '_' || *doc == '.' ||
		  ('A' <= *doc && *doc <= 'Z') || ('0' <= *doc && *doc <= '9') || *doc == '\\') {
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
				lua_type_fmt(&doc, &fmt);
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

char* lua_fmt(const char* doc, char* fmt, int len) {
	const char* docEnd = doc + len;
	char* fmt0         = fmt; // for checking beginning of output
	if (alike(doc, "```lua\n") > 0) {
		fmt = append(fmt, "```lua\n");
		doc += 7;
		if (*doc == '(') {
			switch (doc[1]) {
				case 'g': // '(global)'
					fmt = append(fmt, "_G.");
					break;
			} // other: '(field)'
			doc += 5;
			while (*doc++ > ' ') {}
		}
		lua_code_fmt(&doc, &fmt, "```");
		doc += 3;
		while (*--fmt <= ' ') {}
		fmt = append(fmt + 1, "```\n\n");
		if (doc >= docEnd) return fmt - 1;
	} else doc--;
	int indent[] = {0, 0, 0}; // 1st lvl, 2nd lvl, 1st lvl set text wrap indent
	char kind    = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '[':
				if (doc[1] == ' ') *fmt++ = '\\';
				*fmt++ = '[';
				break;
			case ']':
				if (fmt[-1] == ' ') *fmt++ = '\\';
				*fmt++ = ']';
				break;
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
					break;
				}
			case '<': // code with '<pre>'
				if (*doc != '`' && alike(doc + 1, "pre>") <= 0) *fmt++ = *doc;
				else {
					const char* end = *doc == '`' ? "```" : "</pre>";
					fmt             = append(fmt, "```");
					if (*(doc += (*doc == '`' ? 3 : 5)) != '\n')
						while (*doc > '\n') *fmt++ = *doc++;
					else fmt = append(fmt, "lua");
					if (!*doc) return fmt;
					*fmt++ = *doc++;
					if (alike(fmt - 4, "lua") > 0) lua_code_fmt(&doc, &fmt, end);
					else
						while (!alike(doc, end)) *fmt++ = *doc++;
					while (*--fmt <= ' ') {}
					fmt++;
					doc += end[0] == '`' ? 3 : 5;
					fmt = append(fmt, fmt[-1] == '`' ? "\n```\n" : "```\n\n");
				}
				break;
			case '|': // vim help-page-style links '|text|'
				if (doc[1] != ' ') {
					char* fmtTmp = fmt;
					*fmt++       = '[';
					while (*++doc != '|' && *doc != ' ' && *doc != '\n') *fmt++ = *doc;
					if (*doc != '|') {
						*fmtTmp = '|';
						doc--;
					} else *fmt++ = ']';
				} else *fmt++ = '|';
				break;
			case '~': // 'See:' links deformed into '~https~ //url'
				if (alike(doc + 1, "http") > 0 && (doc[5] == '~' || doc[6] == '~')) {
					if (alike(fmt - 3, " * ") > 0) fmt = append(fmt - 3, "- ");
					doc++;
					*fmt++ = '[';
					while (*doc != '~') *fmt++ = *doc++;
					doc += 2;
					*fmt++ = ':';
					while (*doc != ' ' && *doc != '\n' && *doc) *fmt++ = *doc++;
					doc--;
					*fmt++ = ']';
				} else *fmt++ = '~';
				break;
			case '{': {
				const char* docTmp = doc;
				while (*docTmp != ' ' && *docTmp != '}') *fmt++ = *docTmp++;
				if (*docTmp <= ' ' || docTmp - doc == 1) *fmt++ = *docTmp;
				else { // vim references to params as '{param}'
					fmt[doc - docTmp] = '`';
					*fmt++            = '`';
				}
				doc = docTmp;
			} break;
			case ':':
				if (alike(doc + 1, " ~\n") > 0) { // vim sections ' Section: ~'
					indent[0] = indent[1] = indent[2] = 0;
					int j                             = -1;
					while (fmt > fmt0 && fmt[j] != '\n') j--;
					j += 2;
					fmt[j - 1] = '*';
					if (fmt[j - 3] == '\n') fmt[j - 2] = '*';
					else {
						for (int m = 0; m >= j; m--) fmt[m] = fmt[m - 1];
						fmt++;
					}
					char kind = fmt[j];
					if (kind == 'R') *fmt++ = 's'; // 'Return*s*:'
					fmt = append(fmt, ":**");
					if (kind == 'P') doc += 2;
					else {
						doc += 3;
						*fmt++ = '\n';
						if (kind == 'R') {
							fmt = append(fmt, " - ");
							while (*++doc == ' ') {}
						} else *fmt++ = ' ';
						doc--;
					}
				} else { // highlight other section candidates - capital first letter
					char* fmtTmp = fmt - 1;
					while (fmtTmp > fmt0 && ((*fmtTmp >= 'a' && *fmtTmp <= 'z') || *fmtTmp == '_')) fmtTmp--;
					if (*fmtTmp >= 'A' && *fmtTmp <= 'Z' && // '\n Example:' -> '\n **Example:**'
					  (fmtTmp == fmt0 || fmtTmp[-1] == '\n' || doc[1] == '\n' || alike(fmtTmp - 2, "\n "))) {
						for (int m = fmt - --fmtTmp; m; m--) fmtTmp[m + 2] = fmtTmp[m];
						*++fmtTmp = '*';
						*++fmtTmp = '*';
						fmt       = append(fmt + 2, ":**");
						break;
					}
					*fmt++ = ':';
				}
				break;
			case '@': { // format: '@*param* `name` — desc' - sumneko_ls format
				const char* docTmp = doc + 2;
				while ('a' <= *docTmp && *docTmp <= 'z') docTmp++; // must be a word
				if (*docTmp != '*' || doc[1] != '*' || docTmp[1] != ' ') {
					*fmt++ = '@';
					break;
				}
				doc += 2;
				resolveKind(&doc, &fmt, &kind);
				docTmp = doc;
				if (kind == 'r' || kind == 'p') fmt = append(fmt, " - ");
				if (kind == 'r') {     // '@return'
					if (*++doc == '`') { // fixing word wrapped as variable name
						while (*++doc != '`') *fmt++ = *doc;
						if (alike(doc, "` — ")) doc += 4;
						else { // if no description, go to next line
							while (*++doc > '\n') *fmt++ = *doc;
							doc--;
						}
					} else doc += 2;
					indent[2] += doc - docTmp - 2;
				} else {
					while (*++doc > ' ') *fmt++ = *doc;
					if (alike(doc, " —") > 0) {
						doc += 4;
						*fmt++ = ':';
					}
					while (*doc == ' ') *fmt++ = *doc++;
					indent[2] += --doc - docTmp - 3;
				}
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
				if (((doc[j] == '-' || doc[j] == '+' || doc[j] == '*') &&
				      doc[j + 1] == ' ') || // param description
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
					if (*++doc == -94) {
						doc += 2;
						if (!indent[1]) indent[0] += 2; // `• {}` extra, 2nd lvl is adjusted to that
					}
					lua_param_fmt(&doc, &fmt);              // format param entry
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
			default:
				*fmt++ = *doc;
		}
	}
	return fmt;
}
