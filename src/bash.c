#include "utils.h"

char* bash_fmt(const unsigned char* doc, char* fmt, int len) {
	if (alike((const char*) doc, "``` man\n") > 0) {
		while (doc[len] != '`') len--;
		doc += 6;
		len -= 10;
	}
	const unsigned char* docEnd = doc + len;

	int indent[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, lvl = 0;
	char separated = 0;
	char optlvl    = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '"':
				if (('A' <= doc[1] && doc[1] <= 'Z') || ('a' <= doc[1] && doc[1] <= 'z')) {
					char x = *doc;
					*fmt++ = '*';
					*fmt++ = x;
					while (*++doc && *doc != x) *fmt++ = *doc;
					*fmt++ = x;
					*fmt++ = '*';
				} else *fmt++ = *doc;
				break;
			case '<':
				if (('A' <= doc[1] && doc[1] <= 'Z') || ('a' <= doc[1] && doc[1] <= 'z')) {
					*fmt++ = '*';
					*fmt++ = '<';
					while (*++doc && *doc != '>') *fmt++ = *doc;
					*fmt++ = '>';
					*fmt++ = '*';
				} else *fmt++ = '<';
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
			case '\n':
				*fmt++ = '\n';
				if (doc[1] == '\n') {
					separated = 1;
				} else if (doc[1] == ' ') {
					int i = 0;
					while (*++doc == ' ') i++;
					int ilvl = lvl;
					if (i < indent[lvl] - 3) // 3 to ensure intentional indent
						while (ilvl && i < indent[ilvl] - 3) ilvl--;
					if (i > indent[ilvl] + 3) indent[++ilvl] = i;

					if (separated == 2) *fmt++ = '\n';
					else if (separated == 1) {
						/* if ((' ' < *doc && *doc != '-' && *doc < '0') || ('9' < *doc && *doc < 'A') || ('Z' <
						*doc && *doc < 'a') || 'z' < *doc) { // graph/code text doc -= i; i &= ~1; i -= 2; while
						(*doc > '\n' || (*doc == '\n' && *++doc != '\n')) { for (int j = i; j && *doc == ' ';
						j--) doc++; for (int j = lvl * 2 + 6; j; j--) *fmt++ = ' '; while (*doc > '\n') *fmt++ =
						*doc++; *fmt++ = '\n';
						  }
						  doc--;
						  break;
						} else */
						if (optlvl && fmt[-3] != '`' && fmt[-2] == '\n') fmt--;
					}

					if (ilvl) {
						int j = 0;
						if (separated) {
							const unsigned char* tmp = doc;
							while (*tmp > ' ' || (*tmp > '\n' && (tmp[1] == ' ' || tmp[-1] != ' '))) tmp++;
							if (*tmp == '\n' && *++tmp != '\n')
								for (; *tmp == ' '; tmp++, j++) {}

							if (tmp - doc == 6 || *doc == '-' || (j >= i + 4 && *tmp > '-') || ('0' <= *doc && *doc <= '9')) { // align options with their description
								if (!optlvl) optlvl = ilvl; // ↑ j == indent[ilvl+1] // first option won't appear
								j = 2 + (i = ilvl * 2 - 1);
								while (i-- > 0) *fmt++ = ' ';
								fmt       = append(fmt, *doc == '-' ? "**`" : "- **`");
								char kind = *doc;
								while (*doc > ' ' && *doc != '=') *fmt++ = *doc++; // opt name highlight
								if (kind != '-' && *doc != '=')
									while (*doc > ' ' || (*doc > '\n' && doc[1] != ' ')) *fmt++ = *doc++;
								fmt = append(fmt, "`**");
								if (*doc >= ' ' && doc[1] != ' ') {
									*fmt++ = *doc++;
									*fmt++ = '`'; // rest of the option format
									while (*doc > ' ' || (*doc > '\n' && doc[1] != ' ')) *fmt++ = *doc++;
									fmt = append(fmt, "`:");
								} else *fmt++ = ':';
								if (*doc == '\n' && *++doc > '\n') {
									*fmt++ = '\n';
									while (*++doc == ' ') {}
									while (j-- > 0) *fmt++ = ' ';
									fmt = append(fmt, "- ");
								} else {
									while (*++doc == ' ') {}
									doc--;
								}
							} else if (*doc >= 'a' && *tmp <= '-' && *tmp != ' ' && *tmp > 0) { // example code
								fmt = append(fmt, "```sh\n");
								while (*doc > '\n') *fmt++ = *doc++;
								while (*doc && *++doc > '\n') {
									*fmt++ = '\n';
									for (j = i; j && *doc == ' '; j--) doc++;
									while (*doc > '\n') *fmt++ = *doc++;
								}
								fmt = append(fmt, "```");
								doc--;
							} else if (*tmp <= ' ' || j == i) { // normal paragraph
								for (int i = ilvl * 2 - 1; i; i--) *fmt++ = ' ';
								if (*tmp > 0) fmt = append(fmt, "- "); // wraptext with utf8 '-'
							}
						} else
							for (int i = ilvl * 2 + 1; i; i--) *fmt++ = ' ';
						separated = 0;
					} else { // lower-level header
						*fmt++ = '*';
						*fmt++ = '*';
						while (*doc > '\n') *fmt++ = *doc++;
						fmt       = append(fmt, "**");
						separated = 2;
					}
					if (ilvl < optlvl) optlvl = 0;
					lvl = ilvl;
					doc--;
				} else {
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
				}
				break;
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
