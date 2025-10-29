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
		default: return;
	}
	*fmt++  = *doc;
	*docPtr = doc;
	*fmtPtr = fmt;
}

static void typed_identifier_fmt(const in **docPtr, char **fmtPtr);

/**
 * @brief Format type information to a simpler, better readable format.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void type_fmt(const in **docPtr, char **fmtPtr) {
	static const char *types[] = {
	  "number", "string", "any", "unknown", "integer",  "boolean", "table", "{",
	  "'",      "[[",     "\"",  "[",       "function", "fun(",    "float", "<",
	};
	static const int typeCnt = sizeof(types) / sizeof(const in *);

	const in *doc            = *docPtr;
	char *fmt                = *fmtPtr;
	int wrap                 = 0;
	if (fmt[-1] == '=' || fmt[-1] == ',') *fmt++ = ' ';
	do {
		if (*doc == '|') *fmt++ = *doc++;
		else // do not allow spaces between `|` and types
			while (empty(*doc)) doc++;

		in backTicks = *doc == '`'; // sometimes type is wrapped in backticks
		if (backTicks) doc++;

		while (*doc == '(') { // other times, it is wrapped in ()
			wrap++;
			*fmt++ = *doc++;
		}

		int type = 0, len = 0;
		while (type < typeCnt && !(len = alike(doc, types[type]))) type++;
		if (len > 0) doc += len;
		switch (type) {
			case 0: // number
				*fmt++ = '0';
				*fmt++ = '.';
				break;

			case 1: // string
				if (doc[1] == '=' && (doc[3] == '\'' || doc[3] == '"' || doc[3] == '[')) {
					doc += 2;
					add_string(&doc, &fmt);
					doc++; // TODO: can this be removed?
					while (empty(*doc)) doc++;
				} else {
					*fmt++ = '\'';
					*fmt++ = '\'';
				}
				break;

			case 2: // any
			case 3: // unknown
				*fmt++ = '?';
				break;
			case 4: // integer
				*fmt++ = '0';
				*fmt++ = '.';
				break;
			case 5: // boolean
				if (fmt[-1] == '|' || doc[1] != '=' || (doc[3] != 't' && doc[3] != 'f'))
					fmt = append(fmt, "bool");
				else { // when the variable has a bool value assigned we skip the type declaration
					doc += 3;
					while ('a' <= *doc && *doc <= 'u') *fmt++ = *doc++;
				}
				break;

			case 6: // table
				if (*doc == '<') {
					*fmt++ = '{';
					doc++;
					if (*doc != '>') {
						int len;
						if ((len = alike(doc, "string, "))) {
							doc += len;
							fmt = append(fmt, "[\"\"] = "); // string index -> it's a map/dictionary
						} else if ((len = alike(doc, "integer, ")) || (len = alike(doc, "number, ")))
							doc += len;
						else if ((len = alike(doc, "any, "))) {
							fmt = append(fmt, "? = ");
							doc += len;
						} else fmt = append(fmt, "? = "); // interpret type as value type - no key type

						if (*doc == '>') *fmt++ = '?'; // only indexes specified, no field type
						else type_fmt(&doc, &fmt);     // resolve type of field
					}
					while (*doc && *doc++ != '>') {} // strip all extra useless table data
					*fmt++ = '}';
				} else if (*doc == ' ' && doc[1] == '{') doc++;
				else { // unspecified table `table`
					*fmt++ = '{';
					*fmt++ = '}';
				}
				break;

			case 7: // { - literal table/value definition.
				*fmt++ = '{';
				typed_identifier_fmt(&doc, &fmt);
				while (*doc != '}') *fmt++ = *doc++;
				*fmt++ = *doc++;
				break;

			case 8: // string value
			case 9:
			case 10:
				doc -= len;
				add_string(&doc, &fmt);
				doc++;
				break;

			case 11: // '[' - manually denoted array
				*fmt++ = '{';
				if (*doc != ']') // table/array has data inside
					do {
						if (*doc == ',') *fmt++ = *doc++;
						type_fmt(&doc, &fmt);
					} while (*doc == ',');
				*fmt++ = '}';
				doc++;
				break;

			case 12: // function
				fmt = append(fmt, "fun()");
				break;
			case 13: { // fun():rettype - function with specified args
				fmt = append(fmt, "fun(");
				if (*doc != ')') typed_identifier_fmt(&doc, &fmt);
				*fmt++ = ')';
				if (*doc++ == ')' && *doc == ':') { // fn return value
					if (doc[1] == '.') {
						fmt = append(fmt, ":...");
						doc += 4;
					} else *fmt++ = *doc++;
					type_fmt(&doc, &fmt);
				}
			} break;

			case 14: // float
				*fmt++ = '0';
				*fmt++ = '.';
				*fmt++ = '0';
				break;

			case 15: // generics
				*fmt++ = '<';
				while (isCONST(*doc)) *fmt++ = *doc++;
				if (*doc == ':') {
					*fmt++ = *doc++;
					*fmt++ = ' ';
					type_fmt(&doc, &fmt);
				}

				*fmt++ = *doc++; // should be always >
				break;

			default: // unknown type / value (nil, ...)
				while (isPath(*doc)) {
					if (*doc == '[') break;
					*fmt++ = *doc++;
				}
		}

		while (wrap && *doc == ')') {
			wrap--;
			*fmt++ = *doc++;
		}

		while (*doc == '[' && doc[1] == ']') {
			*fmt++ = *doc++;
			*fmt++ = *doc++;
		}

		if (*doc == '?') { // in table field declarations lua_lsp recognizes ? only after the type
			fmt = append(fmt, "|nil");
			doc++;
		}

		if (backTicks && *doc == '`') doc++;
		while (empty(*doc)) doc++;
	} while (*doc == '|');

	if (*doc == '=') { // also include the actual value
		*fmt++ = ' ';
		*fmt++ = *doc++;
		type_fmt(&doc, &fmt);
	}

	*docPtr = doc;
	*fmtPtr = fmt;
}

/**
 * @brief Format parameter/table name/index and type, or `...`
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void typed_identifier_fmt(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;
	do {                                // parse all args
		if (*doc == ',') *fmt++ = *doc++; // arg separator
		while (empty(*doc)) *fmt++ = *doc++;

		const in *docTmp = doc;
		while (isVar(*doc) || (*doc == '.' && doc[1] != '.')) *fmt++ = *doc++; // arg name
		switch (*doc) {
			case '[': // table fields: fixed index / complex string index

				if (doc[1] == ']') { // not an index, but an array type (`string[]` etc.)
					fmt = fmt - (doc - docTmp);
					doc = docTmp;
					break;
				}

				*fmt++ = *doc++;
				while (*doc != ']') *fmt++ = *doc++;
				*fmt++ = *doc++;   // add the ']'
				if (*doc != ':') { // no type found
					fmt = append(fmt, " = ?");
					break;
				} // fall through to type def
			case ':': // arg type
				doc++;
				*fmt++ = ' ';
				*fmt++ = '=';
				break;

			case '.':
				fmt = append(fmt, "...");
				doc += 3;
				if (*doc == '(') { // table fields: number of left out fields
					while (*doc++ != ')') {}
					continue;   // is last if present -> ends the loop
				} else break; // function: vararg marker

			case '?':
				if (doc[1] != '\n') { // function return: let direct type fall through to default
					fmt = append(fmt, " = nil|");
					doc += 2;
					break;
				}
			default: // there was no var name, it was already the type
				fmt = fmt - (doc - docTmp);
				doc = docTmp;
		}
		type_fmt(&doc, &fmt);

		// TODO: can fn params really result in ...Too Long?
		// while (*doc && (*doc <= ' ' || *doc == '}')) doc++; // 'too long' message fix

		if (alike(doc, "___")) { // param signature markers
			doc += 3;
			fmt = append(fmt, "___");
		}
	} while (*doc == ',');

	*docPtr = doc;
	*fmtPtr = fmt;
}

/// @brief Skips by 9 to get to the function content immediately (to `(`, not over)
static void callable_fmt(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;

	fmt           = append(fmt, "function ");
	doc += 9;

	if (*doc == '(') *fmt++ = '_'; // give a name to unnamed fn (different from anonymous)
	else
		while (*doc != '(' && *doc > ' ') *fmt++ = *doc++; // skip to fn definition
	*fmt++ = *doc++;                                     // append the actual `(`

	typed_identifier_fmt(&doc, &fmt);

	if (*doc != ')' || alike(doc + 1, " end")) {
		*docPtr = doc - 1;
		*fmtPtr = fmt;
		return;
	}
	fmt = append(fmt, ") end");

	if (alike(doc + 1, "\n  ->")) { // ensure we're in the return-type section
		fmt = append(fmt, "\n  =->");
		doc += 6;

		const in *docTmp;
		do {
			while (*doc >= '.') doc++; // skip return value number `2.`
			if (doc[-1] == '.') *fmt++ = ',';
			*fmt++ = *doc++; // add space
			typed_identifier_fmt(&doc, &fmt);
			docTmp = doc;
			while (empty(*doc)) doc++;
		} while ('1' <= *doc && *doc <= '9');

		doc = docTmp[-1] == '\n'
		  ? docTmp - 1
		  : docTmp; // let the newline get parsed normally (or jump back before param_fmt failed)
	} else doc++;
	*docPtr = doc;
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
	const in *doc = *docPtr - 1;
	char *fmt     = *fmtPtr;

	while (!alike(++doc, stop) && *doc) switch (*doc) {
			case 'f': // check for function keyword (definition or anonymous function
				if (doc[-1] == '\n' && (doc[8] == ' ' || doc[8] == '(') && alike(doc + 1, "unction"))
					callable_fmt(&doc, &fmt);
				else *fmt++ = 'f';
				break;

			case '[':              // string [[...]]
				if (doc[1] != '[') { // make sure it's a real string
					*fmt++ = '[';
					break;
				}
			case '"':  // string "..."
			case '\'': // string '...'
				add_string(&doc, &fmt);
				break;

			case '-':
				*fmt++ = '-';
				if (doc[1] == '-') { // lua comment
					*fmt++ = '-';
					doc++;
					if (doc[1] == '[' && doc[2] == '[') {
						doc++;
						add_string(&doc, &fmt);
					} else
						while (*doc > '\n') *fmt++ = *doc++;
				}
				break;

			case ':': // type
				if (doc[1] == '\n') {
					doc += 2;
					while (*doc != '|') doc++;
				}

				if (doc[1] == ' ') {
					if (fmt[-1] == '?') {
						doc--;
						fmt--;
					}
					typed_identifier_fmt(&doc, &fmt);
					doc--;
				} else *fmt++ = *doc; // TODO: can be a member method call (string:gsub())
				break;

			case '\\':
				*fmt++ = '\\';
				*fmt++ = *++doc;
				break;
			default: *fmt++ = *doc;
		}

	*docPtr = doc;
	*fmtPtr = fmt;
}

static void param_fmt_default_value(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	if (*doc == ',' && doc[1] == ' ') doc += 2;
	if (!alike(doc, "default")) return;
	doc += 8;
	char *fmt = append(*fmtPtr, "` = `");
	while (*doc && (*doc != ')' || (doc[1] > ' ' && doc[1] != ':'))) *fmt++ = *doc++;
	*docPtr = doc;
	*fmtPtr = fmt;
}

/**
 * @brief Format a @param list item entry. Does nothing if item is not a param entry.
 * Adds type info from description (like vim's (optional) or default value etc.)
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void param_docs_fmt(const in **docPtr, char **fmtPtr) {
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
		*docPtr = doc;
		*fmtPtr = fmt;
		return;
	}
	if (*doc == '{') doc++;
	if ('A' <= *doc && *doc <= 'Z') return;
	*fmt++ = '`';
	while (('a' <= *doc && *doc <= 'z') || ('A' <= *doc && *doc <= '_') ||
	       ('.' <= *doc && *doc <= '9')) {
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
			*fmt++  = '?';
			*docPtr = doc;
			*fmtPtr = fmt;
			return;
		}
		fmt = append(fmt, " (`");
		doc += 2;
		param_fmt_default_value(&doc, &fmt); // when only default value is given
		type_fmt(&doc, &fmt);
		param_fmt_default_value(&doc, &fmt);
		if (*doc++ != ')' || !*doc || (*doc > ' ' && *doc != ':')) {
			if (ok) *(*fmtPtr)++ = ':'; // param without type info
			return;
		}
		*fmt++ = '`';
		*fmt++ = ')';
		if (*doc == ':') {
			doc++;
			ok = 2;
		}
		if ((ok > 1 ? alike(doc, " (optional)") : alike(doc, " optional"))) {
			doc += ok > 1 ? 11 : 10;
			*fmt++ = '?';
		} else if (!ok && *doc != '\n') return;
	} else if (doc[-1] == '}') doc++;

	if (*doc == ':') doc++;
	*fmt++  = ':';

	*docPtr = doc;
	*fmtPtr = fmt;
}

char *lua_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	const char *fmt0 = fmt; // for checking beginning of output
	fmt              = append(fmt, "```lua\n");
	doc += 7;
	if (*doc == '(') {
		if (alike(doc + 1, "method)")) {
			callable_fmt(&doc, &fmt);
		} else {
			if (doc[1] == 'g') fmt = append(fmt, "_G."); // (global)

			doc += 5;
			while (*doc != ')') doc++;
			doc += 2;
		}
	}
	code_fmt(&doc, &fmt, "```");
	doc += 3;
	while (*--fmt <= ' ') {}
	fmt = append(fmt + 1, "\n```\n\n");

	if (doc >= docEnd) return fmt - 1;
	int indent[] = {0, 0, 0}; // 1st lvl, 2nd lvl, 1st lvl set text wrap indent | deeper levels kept
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
			case '>':
				if (!alike(doc - 1, " >vim\n")) *fmt++ = '>';
				else {
					while (*--fmt <= ' ') {}
					fmt = append(fmt + 1, "\n```vim");
					doc += 4;
					while (*doc && (*doc != '<' || doc[-1] != ' ' || doc[-2] != '\n')) *fmt++ = *doc++;
					fmt = append(fmt - 2, "\n```\n ");
				}
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
				if (*doc != '`' && !alike(doc + 1, "pre>")) *fmt++ = *doc;
				else {
					const char *end = *doc == '`' ? "```" : "</pre>";
					while (*--fmt == ' ') {}
					fmt = append(fmt + 1, "```");
					if (*(doc += (*doc == '`' ? 3 : 5)) != '\n')
						while (*doc > '\n') *fmt++ = *doc++;
					else fmt = append(fmt, "lua");
					if (!*doc) return fmt;
					*fmt++ = *doc++;
					if (alike((in *) fmt - 4, "lua")) code_fmt(&doc, &fmt, end);
					else
						while (!alike(doc, end)) *fmt++ = *doc++;
					while (*--fmt <= ' ') {}
					fmt++;
					doc += end[0] == '`' ? 3 : 5;
					fmt = append(fmt, "\n```\n\n");
				}
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
			case '~': // 'See:' links deformed into '~https~ //url'
				if (alike(doc + 1, "http") && (doc[5] == '~' || doc[6] == '~')) {
					if (alike((in *) fmt - 3, " * ")) fmt = append(fmt - 3, "- ");
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
				const in *docTmp = doc;
				while (*docTmp > ' ' && *docTmp <= '{') *fmt++ = *docTmp++;
				if (*docTmp != '}' || docTmp - doc == 1) *fmt++ = *docTmp;
				else { // vim references to params as '{param}'
					fmt[doc - docTmp] = '`';
					*fmt++            = '`';
				}
				doc = docTmp;
			} break;
			case ':':
				if (alike(doc + 1, " ~\n")) { // vim sections ' Section: ~'
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
					in kind = fmt[j];
					if (kind == 'R') *fmt++ = 's'; // 'Return*s*:'
					fmt = append(fmt, ":**");
					if (kind == 'P') doc += 2; // ': ', '~' is removed with loop iteration
					else {
						doc += 4; // ': ~\n'
						*fmt++ = '\n';
						if (kind == 'R') {
							fmt       = append(fmt, " - ");
							indent[0] = 2; // for list alignment (3 spaces) - make first indent already used
							while (*doc == ' ') {
								indent[1]++;
								doc++;
							}
						} else *fmt++ = ' ';
						doc--;
					}
				} else { // highlight other section candidates - capitalize first letter
					char *fmtTmp = fmt - 1;
					while (fmt0 < fmtTmp && (('a' <= *fmtTmp && *fmtTmp <= 'z') || *fmtTmp == '_')) fmtTmp--;
					if ('A' <= *++fmtTmp && *fmtTmp <= 'Z' // '\n Example:' -> '\n **Example:**'
					    && (fmtTmp == fmt0 || fmtTmp[-1] == '\n' || doc[1] == '\n' ||
					        alike((in *) fmtTmp - 2, "\n ") > 0)) {
						for (int m = fmt - --fmtTmp; m; m--) fmtTmp[m + 2] = fmtTmp[m];
						*++fmtTmp = '*';
						*++fmtTmp = '*';
						fmt       = append(fmt + 2, ":**");
						break;
					}
					*fmt++ = ':';
				}
				break;
			case '@': { // format: '@*param* `name` — desc'
				const in *docTmp = doc + 2;
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
					if (alike(doc, " —")) {
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
				if (alike(++doc, "---\n")) doc += 4;
				int j = 0; // get indentation
				while (doc[j] == ' ') j++;
				if (((doc[j] == '-' || doc[j] == '+' || doc[j] == '*') &&
				     doc[j + 1] == ' ') || // param description
				    alike(doc + j, "• ")) {
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
					param_docs_fmt(&doc, &fmt);             // format param entry
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
