#include "utils.h"

char* bash_fmt(const char* doc, char* fmt, int len) {
	if (alike(doc, "``` man\n") <= 0) { // don't format if not in the expected form of bashls
		while (*doc) *fmt++ = *doc++;
		return fmt;
	} else {
		doc += 6;
		while (doc[len] != '`') len--;
		len -= 4;
	}
	const char* docEnd = doc + len;

	int indent[]       = {8, 0, 0, 0}; // text indent, 1st lvl, 2nd lvl, 1st lvl set text wrap indent
	char separated     = 0;
	char kind          = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '"':
				*fmt++ = '*';
				*fmt++ = '"';
				while (*++doc && *doc != '"') *fmt++ = *doc;
				*fmt++ = '"';
				*fmt++ = '*';
				break;
			case '<':
				*fmt++ = '*';
				*fmt++ = '<';
				while (*++doc > ' ' && *doc != '>') *fmt++ = *doc;
				if (*doc == '>') *fmt++ = '>';
				else doc--;
				*fmt++ = '*';
				break;
			case '-':
				if (fmt[-1] < '0' && (doc[1] == '-' || ('a' <= doc[1] && doc[1] <= 'z')) && doc[2] != '-') {
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
					if (separated == -1) *fmt++ = '\n';
					else if (separated == 1) {
						if (kind == '-') fmt--;
						kind = 0;
					}
					int i = 0;
					while (*++doc == ' ') i++;
					if (i >= 7) {
						int j = 0;
						if (separated) {
							const char* tmp = doc;
							while (*tmp > ' ' || (*tmp > '\n' && (tmp[1] == ' ' || tmp[-1] != ' '))) tmp++;
							if (*tmp == '\n' && *++tmp != '\n')
								for (; *tmp == ' '; tmp++, j++) {}
							if (*tmp == ' ' && tmp - doc == 6) j = -'-';
							else if (*tmp < 0 || j == i) {
								if (!separated) doc -= i - indent[0];
								else doc--;
								separated = 0;
								break;
							} else if (*doc == '-' || (j >= i + 4 && *tmp > '-') || ('A' <= *tmp && *tmp <= 'Z'))
								j = -'-';
							else if (*doc >= 'a') {
								fmt = append(fmt, "```bash\n");
								while (*doc > '\n') *fmt++ = *doc++;
								while (*doc && *++doc > '\n') {
									*fmt++ = '\n';
									for (j = i; j && *doc == ' '; j--) doc++;
									while (*doc > '\n') *fmt++ = *doc++;
								}
								fmt = append(fmt, "```");
								doc -= 2;
								break;
							}
						} else { // TODO: implement a proper text align system (like lua)
							i = (i - indent[0] + 1) / 2;
							while (i-- > 0) *fmt++ = ' ';
						}
						if (j == -'-') { // align options with their description
							kind = '-';
							j    = 2 + (i = (i - indent[0] + 3) / 2);
							while (i-- > 0) *fmt++ = ' ';
							fmt = append(fmt, "- **`");
							while (*doc > ' ' || (*doc > '\n' && doc[1] != ' ')) *fmt++ = *doc++;
							fmt = append(fmt, "`**:");
							if (*doc == '\n' && *++doc > '\n') {
								*fmt++ = '\n';
								while (*++doc == ' ') {}
								while (j-- > 0) *fmt++ = ' ';
								fmt = append(fmt, "- ");
							} else {
								while (*++doc == ' ') {}
								doc--;
							}
						}
						separated = 0;
					}
					doc--;
				} else {
					fmt       = append(fmt, "# ");
					separated = doc[1];
					while (*++doc > '\n') *fmt++ = *doc;
					if (separated == 'S' && fmt[-1] == 'S') { // SYNOPSIS
						fmt = append(fmt, "\n```bash");
						while (*doc && *++doc > '\n') {
							*fmt++ = '\n';
							for (int j = indent[0]; j && *doc == ' '; j--) doc++;
							while (*doc > '\n') *fmt++ = *doc++;
						}
						fmt = append(fmt, "```\n");
					} else separated = -1;
					doc--;
				}
				break;
			case -128: // •
				if (fmt[-1] == -30 && doc[1] == -94) {
					fmt[-1] = '-';
					doc++;
				} else *fmt++ = -128;
				break;
			case '[':
			case '#':
			case '*':
				if (fmt[-1] <= ' ' || doc[1] <= ' ') *fmt++ = '\\';
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
