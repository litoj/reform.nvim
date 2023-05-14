#include "utils.h"

static const char* types[] = {"float", "number", "integer", "string", "any", "unknown", "table",
  "[", "boolean", "function", "fun("};

/**
 * @brief Format type information to a simpler, better readable format.
 *
 * @param doc source string
 * @param fmtPtr ptr to formatted output position
 * @param docPos ptr to source string position
 */
static void lua_type_fmt(const char* doc, char** fmtPtr, int* docPos) {
	int i     = *docPos;
	char* fmt = *fmtPtr;
	do {
		if (doc[i] == '|') *fmt++ = doc[i++];
		if (fmt[-1] == '=' || fmt[-1] == ',') *fmt++ = ' ';
		if (doc[i] <= ' ')
			while (doc[i] <= ' ') i++;
		char mdMod = doc[i] == '`';
		if (mdMod) i++;
		int type = 0, len = 0;
		for (; type < 11; type++, len = 0)
			if ((len = alike(doc + i, types[type]))) break;
		if (doc[i += len] == '[') *fmt++ = '{';
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
				if (doc[i + 1] == '=') i += 3;
				else {
					*fmt++ = '"';
					*fmt++ = '"';
				}
				break;
			case 4: // any
			case 5: // unknown
				*fmt++ = '?';
				break;
			case 6: // table
				if (doc[i] == '<') {
					*fmt++ = '{';
					i++;
					if (doc[i] != '>') {
						for (type = 0; type < 4; type++, len = 0)
							if ((len = alike(doc + i, types[type]))) break;
						if (type < 4 && doc[i + len] != '|') {
							i += len;
							if (type == 3) fmt = append(fmt, "val ="); // string index -> it's a map
						} else {
							*fmt++ = '['; // indexing with a more compicated type
							lua_type_fmt(doc, &fmt, &i);
							fmt = append(fmt, "] =");
						}
						if (doc[i] == '>') { // only indexes specified, no field type
							if (fmt[-1] == '=') *fmt++ = ' ';
							*fmt++ = '?';
						} else { // resolve type of field
							i++;
							lua_type_fmt(doc, &fmt, &i);
						}
					}
					while (doc[i++] != '>') {} // strip all extra useless table data
					*fmt++ = '}';
				} else if (doc[i] == ' ' && doc[i + 1] == '{') i++;
				else { // unspecified table - 'table'
					*fmt++ = '{';
					*fmt++ = '}';
				}
				break;
			case 7: // '[' - manually denoted array
				*fmt++ = '{';
				if (doc[i] != ']') // table/array has data inside
					do {
						if (doc[i] == ',') *fmt++ = doc[i++];
						lua_type_fmt(doc, &fmt, &i);
					} while (doc[i] == ',');
				*fmt++ = '}';
				i++;
				break;
			case 8: // boolean
				fmt = append(fmt, "true|false");
				break;
			case 9: // function
				fmt = append(fmt, "fun()");
				break;
			case 10: // fun( - function with specified args
				fmt = append(fmt, "fun(");
				if (doc[i] != ')') do {
						if (doc[i] == ',') { // arg separator
							*fmt++ = doc[i++];
							*fmt++ = doc[i++];
						}
						while (('a' <= doc[i] && doc[i] <= 'z') || doc[i] == '_' ||
						  ('A' <= doc[i] && doc[i] <= 'Z') || ('0' <= doc[i] && doc[i] <= '9'))
							*fmt++ = doc[i++]; // arg name
						if (doc[i] == ':') { // arg type
							i++;
							*fmt++ = ' ';
							*fmt++ = '=';
							lua_type_fmt(doc, &fmt, &i);
						}
					} while (doc[i] == ',');
				*fmt++ = ')';
				if (doc[i++] == ')' && doc[i] == ':') { // return value
					i++;
					fmt = append(fmt, " -> ");
					lua_type_fmt(doc, &fmt, &i);
				}
				break;
			default: { // unknown type / value (nil, ...)
				char* fmtTmp = fmt;
				while (('a' <= doc[i] && doc[i] <= 'z') || doc[i] == '_' || doc[i] == '.' ||
				  ('A' <= doc[i] && doc[i] <= ']') || ('0' <= doc[i] && doc[i] <= '9'))
					*fmt++ = doc[i++];
				if (doc[i + 1] == '=') { // skip type info and print value only
					i += 3;
					fmt = fmtTmp;
				}
			}
		}
		if (doc[i] == '[' && doc[i + 1] == ']') {
			*fmt++ = '}';
			i += 2;
		}
		if (mdMod && doc[i] == '`') {
			i++;
			while (doc[i] == ' ' || doc[i] == '\n') i++;
		}
	} while (doc[i] == '|');

	*fmtPtr = fmt;
	*docPos = i;
}

/**
 * @brief Format lua code from sumneko_lua markdown docs.
 *
 * @param doc source string
 * @param fmtPtr ptr to formatted output position
 * @param docPos ptr to source string position
 * @param stop '`' or 'p' for <pre> code blocks
 */
static void lua_code_fmt(const char* doc, char** fmtPtr, int* docPos, char stop) {
	char* fmt  = *fmtPtr;
	int i      = *docPos;
	*fmt++     = doc[i];
	int params = 0;
	while (doc[++i]) {
		switch (doc[i]) {
			case '"': // string "..."
				*fmt++ = doc[i];
				while (doc[++i] != '"' && doc[i] != '\n') *fmt++ = doc[i];
				if (doc[i] == '\n') i--;
				else *fmt++ = doc[i];
				break;
			case '\'': // string '...'
				*fmt++ = doc[i];
				while (doc[++i] != '\'' && doc[i] != '\n') *fmt++ = doc[i];
				if (doc[i] == '\n') i--;
				else *fmt++ = doc[i];
				break;
			case '[': // string [[...]]
				*fmt++ = doc[i];
				if (doc[i + 1] != '[') break;
				while (doc[++i] != ']' || doc[i + 1] != ']') *fmt++ = doc[i];
				*fmt++ = doc[i++];
				*fmt++ = doc[i];
				break;
			case '(':
				// check for "function "
				if (fmt[-1] != ' ' || (fmt[-2] == 'n' && fmt[-9] == 'f')) {
					char* fmtTmp = fmt - 1;
					while (*fmtTmp != '\n' && *fmtTmp != ' ') fmtTmp--;
					if (*fmtTmp == ' ' && alike(fmtTmp - 9, "\nfunction")) {
						if (fmt[-1] == ' ') *fmt++ = '_';
						params = 1;
					}
				}
				*fmt++ = '(';
				break;
			case ')':
				*fmt++ = ')';
				if (params) {
					params = 0;
					fmt    = append(fmt, " end");
				}
				break;
			case '>': { // return type
				*fmt++ = '>';
				if (fmt[-2] != '-' || doc[i + 1] != ' ') break;
				int j = 0;
				while (doc[i + j] != ':' && doc[i + j] != '\n') j++;
				if (doc[i + j] == ':') break;
				fmt = append(fmt, " ret");
			}
			case ':': // type
				if (doc[i + 1] == '\n') {
					i += 2;
					while (doc[i] != '|') i++;
				}
				if (doc[i + 1] == ' ') {
					*fmt++ = ' ';
					*fmt++ = '=';
					i++;
					lua_type_fmt(doc, &fmt, &i);
					i--;
				} else *fmt++ = doc[i];
				break;
			case '\\':
				*fmt++ = '\\';
				*fmt++ = doc[++i];
				break;
			case '`':
				if (stop == '`' && doc[i + 1] == '`' && doc[i + 2] == '`') {
					*docPos = i + 2;
					while (*--fmt <= ' ') {}
					fmt++;
				} else *fmt++ = doc[i];
				break;
			case '<':
				if (stop == 'p' && alike(doc + i + 1, "/pre>")) {
					*docPos = i + 5;
					while (*--fmt <= ' ') {}
					fmt++;
					break;
				}
			case '-':
				if (doc[i + 1] == '-') {
					while (doc[i] != '\n') *fmt++ = doc[i++];
					*fmt++ = '\n';
				}
			default:
				*fmt++ = doc[i];
		}
	}
	*fmtPtr = fmt;
	*docPos = i;
}

/**
 * @brief Format a list item entry as a parameter. Does nothing if item is not a param entry.
 *
 * @param doc source string
 * @param fmtPtr ptr to formatted output position
 * @param docPos ptr to source string position
 */
static void lua_param_fmt(const char* doc, char** fmtPtr, int* docPos) {
	int i     = *docPos;
	char* fmt = *fmtPtr;
	*fmt++    = '-';
	*fmt++    = ' ';
	*fmtPtr   = fmt;
	if (doc[i += 2] == -94) i += 2;
	*docPos = i;

	if (doc[i] == '|') return;
	else if (doc[i] == '"') {
		*fmt++ = '*';
		*fmt++ = '"';
		while (doc[++i] != '"') *fmt++ = doc[i];
		*fmt++ = '"';
		*fmt++ = '*';
	} else {

		if (doc[i] == '{') i++;
		*fmt++ = '`';
		while (('a' <= doc[i] && doc[i] <= 'z') || doc[i] == '_' || doc[i] == '.' ||
		  ('A' <= doc[i] && doc[i] <= 'Z') || ('0' <= doc[i] && doc[i] <= '9') || doc[i] == '\\') {
			if (doc[i] == '\\') i++;
			*fmt++ = doc[i++];
		}
		*fmt++ = '`';
		if (doc[i] == '}') i++;
		else if (doc[i] == ':' ? doc[i + 1] != ' ' : doc[i] != ' ' || doc[i + 1] != '(')
			return; // it's not a param

		if (doc[i] == ' ' && doc[i + 1] == '(') { // contains type info
			fmt = append(fmt, " = *`");
			i += 2;
			lua_type_fmt(doc, &fmt, &i);
			if (doc[i++] != ')' || (doc[i] != ':' && doc[i] > '\n'))
				return; // it's not type info of a param
			*fmt++ = '`';
			*fmt++ = '*';
		} else if (doc[i - 1] == '}') i++;

		*fmt++ = ':';
		if (doc[i] == ':') i++;
	}

	*fmtPtr = fmt;
	*docPos = i;
}

char* lua_fmt(const char* doc, char* fmt, int len) {
	int i = 0;
	if (alike(doc, "```lua\n")) {
		fmt = append(fmt, "```lua\n");
		i += 7;
		if (doc[i] == '(') {
			switch (doc[i + 1]) {
				case 'g': // '(global)'
					fmt = append(fmt, "_G.");
					break;
			} // other: '(field)'
			i += 5;
			while (doc[i++] != ' ') {}
		}
		lua_code_fmt(doc, &fmt, &i, '`');
		fmt = append(fmt, "\n```");
		if (!doc[i]) return fmt - 2;
	} else {
		while (doc[i] != '\n' && doc[i]) *fmt++ = doc[i++];
		*fmt++ = doc[i];
	}
	int indent[] = {0, 0}; // [0] = base, [1] = last noted
	char kind    = 0;
	while (++i < len) {
		switch (doc[i]) {
			case '[':
				if (doc[i + 1] == ' ') *fmt++ = '\\';
				*fmt++ = '[';
				break;
			case ']':
				if (fmt[-1] == ' ') *fmt++ = '\\';
				*fmt++ = ']';
				break;
			case '\\':
				*fmt++ = doc[i++];
				*fmt++ = doc[i];
				break;
			case '`': // code with '```'
				if (doc[i + 1] == '`' && doc[i + 2] == '`') {

					while (*--fmt != '\n') {}
					fmt = append(fmt + 1, "```");
					if (doc[i += 3] != '\n')
						while (doc[i] != '\n') *fmt++ = doc[i++];
					else fmt = append(fmt, "lua");
					*fmt++ = doc[i++];
					if (alike(fmt - 4, "lua")) lua_code_fmt(doc, &fmt, &i, '`');
					else {
						while (!alike(doc + i, "```")) *fmt++ = doc[i++];
						i += 3;
					}
					fmt = append(fmt, "\n```");

				} else {
					*fmt++ = '`';
					while (doc[++i] != '`' && doc[i] != '\n' && doc[i]) *fmt++ = doc[i];
					*fmt++ = '`';
					if (doc[i] != '`') i--;
				}
				break;
			case '<': // code with '<pre>'
				if (alike(doc + i + 1, "pre>")) {

					while (*--fmt != '\n') {}
					fmt = append(fmt + 1, "```");
					if (doc[i += 5] != '\n')
						while (doc[i] != '\n') *fmt++ = doc[i++];
					else fmt = append(fmt, "lua");
					*fmt++ = doc[i++];
					if (alike(fmt - 4, "lua")) lua_code_fmt(doc, &fmt, &i, 'p');
					else {
						while (!alike(doc + i, "</pre>")) *fmt++ = doc[i++];
						i += 5;
					}
					fmt = append(fmt, "\n```");

				} else *fmt++ = doc[i];
				break;
			case '|': // vim help-page-style links '|text|'
				if (doc[i + 1] != ' ') {
					char* fmtTmp = fmt;
					*fmt++       = '[';
					while (doc[++i] != '|' && doc[i] != ' ' && doc[i] != '\n') *fmt++ = doc[i];
					if (doc[i] != '|') {
						*fmtTmp = '|';
						i--;
					} else *fmt++ = ']';
				} else *fmt++ = '|';
				break;
			case '~': // 'See:' links deformed into '~https~ //url'
				if (alike(doc + i + 1, "http") && (doc[i + 5] == '~' || doc[i + 6] == '~')) {
					if (alike(fmt - 3, " * ")) fmt = append(fmt - 3, "- ");
					i++;
					*fmt++ = '[';
					while (doc[i] != '~') *fmt++ = doc[i++];
					i += 2;
					*fmt++ = ':';
					while (doc[i] != ' ' && doc[i] != '\n' && doc[i]) *fmt++ = doc[i++];
					i--;
					*fmt++ = ']';
				} else *fmt++ = '~';
				break;
			case '{': {
				const char* docTmp = doc + i;
				while (*docTmp != ' ' && *docTmp != '}') *fmt++ = *docTmp++;
				if (*docTmp <= ' ') *fmt++ = *docTmp;
				else { // vim references to params as '{param}'
					fmt[doc + i - docTmp] = '`';
					*fmt++                = '`';
				}
				i = docTmp - doc;
			} break;
			case ':':
				if (alike(doc + i + 1, " ~\n")) { // vim sections ' Section: ~'
					int j = 0;
					while (fmt[-j] != '\n') j++; // can crash if section is the first recorded word (never)
					j -= 2;
					fmt[-j - 1] = '*';
					if (fmt[-j - 3] == '\n') fmt[-j - 2] = '*';
					else {
						for (int m = 0; m <= j; m++) fmt[-m] = fmt[-m - 1];
						fmt++;
					}
					char kind = fmt[-j];
					if (kind == 'R') *fmt++ = 's'; // 'Return*s*:'
					fmt = append(fmt, ":**");
					if (kind == 'P') i += 2;
					else {
						i += 3;
						*fmt++ = '\n';
						if (kind == 'R') {
							fmt = append(fmt, " - ");
							while (doc[++i] == ' ') {}
						} else *fmt++ = ' ';
						i--;
					}
				} else {
					char* fmtTmp = fmt - 1;
					while ((*fmtTmp >= 'a' && *fmtTmp <= 'z') || *fmtTmp == '_') fmtTmp--;
					if (*fmtTmp >= 'A' && *fmtTmp <= 'Z' && // '\n Example:' -> '\n **Example:**'
					  (fmtTmp[-1] == '\n' || doc[i + 1] == '\n' || alike(fmtTmp - 2, "\n "))) {
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
				int j     = 2;
				indent[0] = 0;
				indent[1] = 3;
				while (doc[i + j] >= 'a' && doc[i + j] <= 'z') j++; // must be a word
				if (doc[i + j] != '*' || doc[i + 1] != '*' || doc[i + j + 1] != ' ') {
					*fmt++ = '@';
					break;
				}
				i += 2;
				fmt = resolveKind(doc, fmt, &i, &kind);
				if (kind == 'r' || kind == 'p') fmt = append(fmt, " - ");
				if (kind == 'r') {       // '@return'
					if (doc[++i] == '`') { // fixing word wrapped as variable name
						while (doc[++i] != '`') *fmt++ = doc[i];
						i += 4;
					} else i += 2;
				} else {
					j = i;
					while (doc[++i] != ' ') *fmt++ = doc[i];
					if (doc[i + 1] == -30 && doc[i + 3] == -108) { // —
						i += 4;
						*fmt++    = ':';
						indent[0] = -1;
						indent[1] = i - j - 2;
					}
					*fmt++ = ' ';
				}
			} break;
			case '\n': {
				if (fmt[-1] == ' ') fmt--;
				if (fmt[-1] != '\n') *fmt++ = '\n';
				if (alike(doc + i + 1, "---\n")) i += 4;
				int j = 1; // get beginning of text on the next line
				while (doc[i + j] == ' ') j++;
				if (((doc[i + j] == '-' || doc[i + j] == '+' || doc[i + j] == '*') &&
				      doc[i + j + 1] == ' ') || // param description
				  alike(doc + i + j, "• ")) {
					if (j > 4) { // 2nd+ level indent
						if (indent[0] > 0 && indent[0] <= j) i += indent[1];
						else i += indent[1] = (indent[0] = j) - 4; // 1st occurence of 2nd level indent
					} else {                                     // back at 1st level
						indent[0] = 0;
						indent[1] = doc[i + j] == -30 ? 3 : 0; // indent only for '   • {param}'
						if (j == 4) i += j - 2;
					}
					while (doc[++i] == ' ') *fmt++ = ' ';
					lua_param_fmt(doc, &fmt, &i);
					i--;
				} else if (indent[0] == -1 && j > 3) {
					for (int k = indent[1]; k > 0; k--) *fmt++ = ' ';
					i += j - 1;
				} else if (indent[1] < j) i += indent[1]; // wrapped text alignment
			} break;
			default:
				*fmt++ = doc[i];
		}
	}
	if (fmt[-1] != '\n') *fmt++ = '\n';
	return fmt;
}
