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

/**
 * @brief Parse elipsis code occurence and determine meaning.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @return if elipsis indicates end of values for the current block or just a vararg marker
 */
static _Bool elipsis(const in **docPtr, char **fmtPtr) {
	*fmtPtr = append(*fmtPtr, "...");
	*docPtr += (*docPtr)[3] == '.' ? 4 : 3;
	if (**docPtr == '(') { // table fields: number of left out fields
		const in *doc = *docPtr;
		while (*doc++ != ')') {}
		// catch case of: ...(too long)...<some leftover text>
		if (alike(doc, "...")) { // skip variable name that was left over behind the ...()...
			// XXX: lua_ls always puts 8 last chars from the left out part.
			doc += 3 + 8;
		}
		*docPtr = doc;
		return 1; // is last if present -> ends the loop
	} else return 0; // function: vararg marker
}

static void typed_identifier_fmt(const in **docPtr, char **fmtPtr, char spacer);

/**
 * @brief Format type information to a simpler, better readable format.
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 */
static void type_fmt(const in **docPtr, char **fmtPtr) {
	static const char *types[] = {"number", "string", "any", "unknown", "integer", "boolean",
	                              "table",  "{",      "'",   "[[",      "\"",      "function",
	                              "fun(",   "float",  "<",   "("};
	static const int typeCnt   = sizeof(types) / sizeof(const in *);

	const in *doc              = *docPtr;
	char *fmt                  = *fmtPtr;
	if (fmt[-1] == '=' || fmt[-1] == ',') *fmt++ = ' ';
	do {
		if (*doc == '|') *fmt++ = *doc++;
		else // do not allow spaces between `|` and types
			while (empty(*doc)) doc++;

		in backTicks = *doc == '`'; // sometimes type is wrapped in backticks
		if (backTicks) doc++;

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
						else type_fmt(&doc, &fmt); // resolve type of field
					}
					while (*doc && *doc++ != '>') {} // strip all extra useless table data
					*fmt++ = '}';
				} else if (*doc == ' ' && doc[1] == '{') doc++;
				else { // unspecified table `table`
					*fmt++ = '{';
					*fmt++ = '}';
				}
				break;

			case 7: { // { - literal table/value definition.
				*fmt++      = '{';
				char spacer = *doc;
				typed_identifier_fmt(&doc, &fmt, spacer);

				if (*doc == '}')
					if (fmt[-1] == *--doc) fmt--;

				if (spacer > ' ') spacer = *doc; // there was no spacer, so make the check pass
				else *fmt++ = spacer;

				// for elipsis fallback just assume it was supposed to end already
				if (*doc == spacer && doc[1] == '}') doc += 2;
				*fmt++ = '}';
			} break;

			case 8: // string value
			case 9:
			case 10:
				doc -= len;
				add_string(&doc, &fmt);
				doc++;
				break;

			case 11: // function
				fmt = append(fmt, "fun()");
				break;
			case 12: { // fun():rettype - function with specified args
				fmt = append(fmt, "fun(");
				if (*doc != ')') typed_identifier_fmt(&doc, &fmt, ' ');
				*fmt++ = ')';
				if (*doc++ == ')' && *doc == ':') { // fn return value
					if (doc[1] == '.') {
						fmt = append(fmt, ":...");
						doc += 4;
					} else *fmt++ = *doc++;
					type_fmt(&doc, &fmt);
				}
			} break;

			case 13: // float
				*fmt++ = '0';
				*fmt++ = '.';
				*fmt++ = '0';
				break;

			case 15: // wrapped in ()
				*fmt++ = '(';
				type_fmt(&doc, &fmt);
				*fmt++ = *doc++; // ')'
				break;

			default: // unknown type / value (nil, ...)
				while (isVar(*doc) || (*doc == '.' && doc[1] != '.') || *doc == '-') {
					if (*doc == '[') break;
					*fmt++ = *doc++;
				}
				if (fmt[-1] == '.') { // get back before the elipsis
					fmt -= 3;
					doc -= 3;
				}

				if (*doc != '<') break;
				else doc++; // NOTE: intentional fallthrough for generics

			case 14: { // generics
				*fmt++ = '<';
				typed_identifier_fmt(&doc, &fmt, '\0');
				*fmt++ = *doc++; // should be always >
			} break;
		}

		if (*doc == '.' && doc[1] == '.' && doc[2] == '.') elipsis(&doc, &fmt);

		while (*doc == '[' && doc[1] == ']') { // annotate an array type
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

	// also include the actual value
	if (*doc == '=' || *doc == '{') {
		*fmt++ = ' ';
		*fmt++ = '=';
		if (*doc == '=') doc++;
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
 * @param spacer ' ' or '\n' (or any char if there is no spacer) that follows the sep (after ','),
 * or '\0' if not known, used to recover from elipsis
 */
static void typed_identifier_fmt(const in **docPtr, char **fmtPtr, char spacer) {
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;
	do { // parse all args
		if (*doc == ',') {
			if (!spacer) spacer = doc[1];
			else if (spacer <= ' ' ? doc[1] != spacer : doc[1] <= ' ') break;

			*fmt++ = *doc++; // arg separator
		}
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
				type_fmt(&doc, &fmt);
				if (*doc != ']') { // aftereffects of elipsis
					*fmt++ = ']';
					continue;
				} else *fmt++ = *doc++; // add the ']'

				if (*doc != ':') { // no type found
					fmt = append(fmt, " = ?");
					continue;
				} // fall through to type def
			case ':': // arg type
				doc++;
				*fmt++ = ' ';
				*fmt++ = '=';
				break;

			case '.': // we know it's an elipsis from the loop before
				if (elipsis(&doc, &fmt)) continue;
				else break; // parse the type after it

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
	*fmt++ = *doc++; // append the actual `(`

	typed_identifier_fmt(&doc, &fmt, '\0'); // we don't know if there are spaces between the params

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
			typed_identifier_fmt(&doc, &fmt, '-'); // any spacer, because it won't find sep (',')
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
	const in *doc = *docPtr;
	char *fmt     = *fmtPtr;

	if (*stop == '`' && *doc == '(') {
		if (alike(doc + 1, "method)")) callable_fmt(&doc, &fmt);
		else {
			if (doc[1] == 'g') fmt = append(fmt, "_G."); // (global)
			else {
				/* while (*doc != ')') *fmt++ = *doc++;
				 *fmt++ = ')';
				 *fmt++ = '.'; */
			}

			while (*doc != ')') doc++;
			doc += 2; // ') '
		}
	}

	doc--;
	while (!alike(++doc, stop) && *doc) switch (*doc) {
			case 'f': // check for function keyword (definition or anonymous function
				if (doc[-1] == '\n' && (doc[8] == ' ' || doc[8] == '(') && alike(doc + 1, "unction"))
					callable_fmt(&doc, &fmt);
				else *fmt++ = 'f';
				break;

			case '[': // string [[...]]
				if (doc[1] != '[') { // make sure it's a real string
					*fmt++ = '[';
					break;
				}
			case '"': // string "..."
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
					while (*doc && *doc != '|') doc++;
					if (!*doc) {
						fmt     = append(fmt, ERROR_STR);
						*docPtr = doc;
						*fmtPtr = fmt;
						return;
					}
				}

				if (doc[1] == ' ') {
					if (fmt[-1] == '?') {
						doc--;
						fmt--;
					}
					// pass in a _no-space_ spacer to not parse potential following values
					typed_identifier_fmt(&doc, &fmt, '-');
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
static int param_docs_fmt(const in **docPtr, char **fmtPtr) {
	const in *doc = *docPtr;
	if (*doc == '|') return 0;
	int offset = 0;
	if (*doc == '{') {
		doc++;
		offset += 2; // also account for the closing brace
	}
	if ('A' <= *doc && *doc <= 'Z') return 0; // PascalCase is only for classes, not var_names

	char *fmt = *fmtPtr;

	*fmt++    = '`';
	if (*doc == '"') {
		*fmt++ = *doc++;
		while (*doc >= ' ' && *doc != '"') *fmt++ = *doc++;
		*fmt++ = *doc++;
	} else {
		while (isPath(*doc)) {
			if (*doc == '\\') doc++;
			*fmt++ = *doc++;
		}
	}
	*fmt++ = '`';

	if (fmt[-2] == '"') { // no type info or optionality expected for strings
		if (*doc == ':') *fmt++ = *doc++;
		*docPtr = doc;
		*fmtPtr = fmt;
		return 0;
	}
	char end = *doc;
	if (end == '}') doc += 1 + (doc[2] == ' '); // ensure we're at the next ' '
	else if (end == ':') doc++; // sometimes appears only after the type info
	else end = 0;

	if (*doc != ' ') {
		if (end != ':' && *doc != ':') return 0; // not a param

		*fmt++  = end == ':' ? end : *doc++;
		*docPtr = doc;
		*fmtPtr = fmt;
		return offset;
	}

	if (doc[1] == '(') { // contains type info
		doc += 2;
		int match = alike(doc, "optional)");
		if (match) {
			*fmt++ = '?';
			doc += match;
		} else {
			if (end) { // make a checkpoint before " ("
				*docPtr = doc - 2;
				*fmtPtr = fmt;
			}
			fmt = append(fmt, " (`");
			param_fmt_default_value(&doc, &fmt); // when only the default value is given
			type_fmt(&doc, &fmt);
			param_fmt_default_value(&doc, &fmt);
			if (*doc++ != ')' || !*doc || (*doc > ' ' && *doc != ':')) { // it was just a normal comment
				if (end) *(*fmtPtr)++ = ':'; // param without type info
				return offset;
			}
			*fmt++ = '`';
			*fmt++ = ')';
			if (*doc == ':') {
				doc++;
				if ((match = alike(doc, " (optional)"))) { // can appear separately from the type info
					doc += match;
					*fmt++ = '?';
				}
				end = ':';
			} else {
				if ((match = alike(doc, " optional"))) {
					doc += match;
					*fmt++ = '?';
				}
			}

			if (!end && *doc != '\n' && *doc != ':') return offset;
		}
	} else if (end == '}') doc++;
	else if (doc[1] == ':') {
		offset++;
		doc++;
	} else if (end != ':' && *doc != ':') {
		return 0;
	}

	if (*doc == ':') doc++;
	*fmt++  = ':';

	*docPtr = doc;
	*fmtPtr = fmt;
	return offset;
}

char *lua_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	const char *fmt0 = fmt; // for checking beginning of output

	int indent[]     = {
    -1, -1, -1
  }; // 1st lvl, 2nd lvl, 1st lvl set text wrap indent | deeper levels kept
	char kind = 0;
	doc--;
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
					while (++doc < docEnd && *doc != '`') *fmt++ = *doc;
					*fmt++ = '`';
					if (*doc != '`') doc--;
					break;
				}
			case '<': // code with '<pre>'
				if (*doc != '`' && !alike(doc + 1, "pre>")) *fmt++ = *doc;
				else {
					const char *end = *doc == '`' ? "```" : "</pre>";
					if (fmt > fmt0 + 1 && fmt[-2] != '\n') *fmt++ = '\n';
					while (--fmt > fmt0 && *fmt == ' ') {}
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
				if (doc[1] > ' ') {
					char *fmtTmp = fmt;
					*fmt++       = '[';
					while (*++doc > ' ' && *doc != '|') *fmt++ = *doc;
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
					indent[0] = indent[1] = indent[2] = -1;
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
							indent[1] = 0;
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
				if (kind == 'r' || kind == 'p') fmt = append(fmt, " - ");

				if (doc[1] == '`') { // varname
					doc++;
					while (*doc > ' ') *fmt++ = *doc++;
				}

				if (alike(doc, " —")) { // separation from comment
					doc += 4;
					if (fmt[-2] != '-') *fmt++ = ':'; // there was no varname, so skip spaces
					else doc++;
					if (*doc == ':') doc += 2; // desc with special char is a leftover from retval name
				}

				while (*doc == ' ') *fmt++ = *doc++;
				doc--;

				if (kind) {
					indent[0] = 0; // we don't know the type so we can't adjust text indent properly
					indent[1] = -1; // reset deeper level indent
					                // indent[2] = --doc - docTmp - 3; // TODO: adjust this
				} // else indent[2] = --doc - docTmp - 3;
			} break;
			case '\n': {
				if (fmt > fmt0 && fmt[-1] == ' ') fmt--;
				if (fmt > fmt0 && fmt[-1] != '\n') *fmt++ = '\n';
				if (alike(++doc, "---\n")) doc += 4;

				int j = 0; // get indentation
				while (doc[j] == ' ') j++;

				if (((doc[j] == '-' || doc[j] == '+' || doc[j] == '*') && doc[j + 1] == ' ') ||
				    alike(doc + j, "• ")) { // list item and/or param description

					if (indent[0] == -1 || j <= indent[0]) { // update 1st level indent to lower value
						indent[1] = indent[2] = -1;
						doc += indent[0]      = j;
						*fmt++                = ' ';
					} else {
						if (indent[1] == -1 || j < indent[1]) indent[1] = j; // update 2nd lvl indent

						doc += indent[1]; // align to 2nd lvl
						fmt = append(fmt, "   ");
					}
					while (*doc++ == ' ') *fmt++ = ' '; // keep alignment of the rest of the text

					if (*++doc == 162) { // `• {}` extra
						doc += 2;
						if (indent[1] == -1) indent[0] += 2; // update 1st lvl for 2nds lvl to skip the diff
					}

					*fmt++ = '-';
					*fmt++ = ' ';

					// format param entry
					int desc_indent_offset = param_docs_fmt(&doc, &fmt);
					// update offset according to param name formatting (extra space before ':' / {} etc.)
					// there is always only one
					if (desc_indent_offset) {
						if (j == indent[0]) indent[2] = indent[0] + desc_indent_offset;
						else if (j == indent[1]) indent[2] = -indent[1] - desc_indent_offset;
					} else indent[2] = -1;
				} else if (indent[0] != -1 && j >= indent[0]) { // wrapped text alignment

					if (indent[1] != -1 && j >= indent[1]) { // deeper than (or at) 2nd lvl, not aligning
						if (indent[2] < -1 && j >= -indent[2]) doc -= indent[2];
						else doc += indent[1];
						fmt = append(fmt, "   ");
					} else if (indent[0]) { // were at the 1st lvl -> do alignment
						doc += indent[(indent[2] > -1 && j >= indent[2]) ? 2 : 0];
						*fmt++ = ' ';
					}
					while (*doc == ' ') *fmt++ = *doc++;
				}
				doc--;
			} break;
			case '\'':
			case '"':
				if (doc[1] <= ' ' || isVar(doc[-1])) {
					*fmt++ = *doc;
				} else {
					*fmt++ = '`';
					add_string(&doc, &fmt);
					*fmt++ = '`';
				}
				break;
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
