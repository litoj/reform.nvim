#include "utils.h"

char *bash_fmt(const in *doc, char *fmt, int len) {
	while (doc[len] != '`') len--;
	doc += 6;
	len -= 10;
	const in *docEnd = doc + len;

	int indent[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, lvl = 0;
	char separated = 0;
	char optlvl    = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '"':
			case '>':
				if (('A' <= doc[1] && doc[1] <= 'Z') || ('a' <= doc[1] && doc[1] <= 'z')) {
					char x = *doc;
					*fmt++ = '*';
					*fmt++ = x;
					if (x == '<') x = '>';
					while (*++doc && *doc != x) *fmt++ = *doc;
					*fmt++ = x;
					*fmt++ = '*';
				} else *fmt++ = *doc;
				break;
			case '-':
				if (fmt[-1] < '0' && 'a' <= doc[doc[1] == '-' ? 2 : 1]) {
					*fmt++ = '`';
					while (*doc >= '0' || *doc == '-') *fmt++ = *doc++;
					*fmt++ = '`';
					doc--;
				} else *fmt++ = '-';
				break;
			case '$':
				*fmt++ = '`';
				*fmt++ = '$';
				doc++;
				while (*doc >= '0') *fmt++ = *doc++;
				*fmt++ = '`';
				doc--;
				break;
			case '\n': {
				*fmt++ = '\n';
				if (doc[1] == '\n') {
					separated = 1;
					break;
				} else if (doc[1] != ' ') { // section label
					fmt       = append(fmt, "# ");
					separated = doc[1];
					while (*++doc > '\n') *fmt++ = *doc;
					if ((separated == 'S' && fmt[-1] == 'S') || (separated == 'N' && fmt[-1] == 'E')) {
						// SYNOPSIS, NAME
						if (separated == 'S') fmt = append(fmt, "\n```sh");
						else *fmt++ = '\n';
						while (*doc && *++doc > '\n') {
							*fmt++ = '\n';
							for (int i = 7; *doc == ' '; i = 7) {
								while (i-- && *doc == ' ') doc++;
								if (i == -1 && *doc == ' ') *fmt++ = ' ';
							}
							while (*doc > '\n') *fmt++ = *doc++;
						}
						if (separated == 'S') fmt = append(fmt, "```\n");
						else doc--;
					} else separated = 2;
					doc--;
					break;
				}

				// item/option, paragraph and code parser/aligner
				int i = 0;
				while (*++doc == ' ') i++;
				int ilvl = lvl;
				if (i < indent[lvl] - 3) // 3 to ensure intentional indent
					while (ilvl && i < indent[ilvl] - 3) ilvl--;
				if (i > indent[ilvl] + 3) indent[++ilvl] = i;

				if (separated == 2) *fmt++ = '\n'; // label separation
				else if (separated == 1) {
					if (optlvl && fmt[-3] != '`' && fmt[-2] == '\n') fmt--;
				}

				if (!separated) { // paragraph continuation
					for (int i = ilvl * 2 + 1; i; i--) *fmt++ = ' ';
					separated = 0;
					if (ilvl < optlvl) optlvl = 0;
					lvl = ilvl;
					doc--;
					break;
				} else if (!ilvl) { // lower-level header
					*fmt++ = '*';
					*fmt++ = '*';
					while (*doc > '\n') *fmt++ = *doc++;
					fmt       = append(fmt, "**");
					separated = 2;
					lvl = optlvl = 0;
					doc--;
					break;
				}

				int j                    = 0;
				const in *tmp = doc;
				int kind                 = 0;
				// secondary text kind determination
				if (('A' <= *tmp && *tmp <= 'Z') || ('a' <= *tmp && *tmp <= 'z')) {
					tmp++;
					while ('a' <= *tmp && *tmp <= 'z') tmp++;
					kind = *tmp <= ' ' ? 'p' : 'o';
				} else if (*tmp < '0' || ('9' < *tmp && *tmp < 127)) kind = *tmp == '-' ? 'o' : 'c';

				while (*tmp > ' ' ||
				       (*tmp == ' ' &&
				        (tmp[1] == ' ' ||
				         (tmp[-1] != ' ' && // detect sentence start " *[A-Z][a-z]\+"
				          (tmp - doc != 6 || (' ' <= tmp[1] && tmp[1] < 'A') || 'Z' < tmp[1] ||
				           tmp[2] < 'a' || 'z' < tmp[2])))))
					tmp++;
				if (tmp[-1] != ':' && *tmp == '\n' && *++tmp != '\n') // check for item of this line
					for (; *tmp == ' '; tmp++, j++) {}

				// primary text kind determination
				if (
				  tmp - doc == 6 // using indent (8) - separation space from desc - last char
				  || (j >= i + 4 && *tmp > '-' && *tmp != ':') // next line is a continuation of this
				)
					kind = 'o';
				else if (tmp - doc < 70 && 'Z' < *doc && *doc < 127 && (j == i // aligned short commands
						|| (*tmp == '\n' && (*doc < 'a' || 'z' < *doc)))) // single longer command
					kind = 'c';
				else if (*tmp <= ' ' || *doc > 127 || (j == i && !kind)) kind = 'p';

				if (kind == 'o') {            // - **`option`** `extras`: Description
					if (!optlvl) optlvl = ilvl; // ↑ j == indent[ilvl+1] // first option won't appear
					j               = 2 + (i = ilvl * 2 - 1);
					char *lineStart = fmt;
					while (i-- > 0) *fmt++ = ' ';
					char kind = *doc;
					fmt       = append(fmt, kind == '-' ? "**`" : "- **`");
					while (*doc > ' ' && *doc != '=') *fmt++ = *doc++; // opt name highlight
					fmt = append(fmt, "`**");
					if (*doc >= ' ' && doc[1] != ' ' && doc < tmp) { // rest of the option format
						*fmt++ = *doc++;
						*fmt++ = '`';
						while (*doc > ' ' || (doc<tmp && * doc> '\n' && doc[1] != ' ')) *fmt++ = *doc++;
						fmt = append(fmt, "`:");
					} else *fmt++ = ':';
					if (*doc == '\n' && *++doc > '\n') { // description on the next line
						while (*++doc == ' ') {}
						int remaining = lineStart + 79 - fmt;
						lineStart     = fmt;
						tmp           = doc;
						*fmt++        = ' '; // try to describe right after
						while (remaining-- > 0 && *doc > '\n') *fmt++ = *doc++;
						if (*doc > '\n') { // didn't fit
							fmt    = lineStart;
							doc    = tmp;
							*fmt++ = '\n'; // just adjust alignment to keep textwidth
							while (j-- > 0) *fmt++ = ' ';
							fmt = append(fmt, "- ");
						}
					} else {
						while (*++doc == ' ') {}
						doc--;
					}
				} else if (kind == 'c') { // example code
					fmt = append(fmt[-2] == '\n' && separated != 2 ? fmt - 1 : fmt, "```sh\n");
					while (*doc > '\n') *fmt++ = *doc++;
					while (*doc && *++doc > '\n') {
						*fmt++ = '\n';
						for (j = i; j && *doc == ' '; j--) doc++;
						while (*doc > '\n') *fmt++ = *doc++;
					}
					fmt = append(fmt, "```");
					doc--;
				} else if (kind == 'p') { // normal paragraph
					if (fmt[-2] == '\n' && separated != 2) fmt--;
					for (int i = ilvl * 2 - 1; i; i--) *fmt++ = ' ';
					if (*tmp > 0) fmt = append(fmt, "- ");
				}
				separated = 0;
				if (ilvl < optlvl) optlvl = 0;
				lvl = ilvl;
				doc--;
			} break;
			case 128: // •
				if (fmt[-1] == -30 && doc[1] == 162) {
					doc += 2;
					while (*doc == ' ') doc++;
					fmt--;
					doc--;
				} else *fmt++ = -128;
				break;
			case '[':
			case '#':
			case '*': *fmt++ = '\\';
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
