#include "utils.h"

char *csharp_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	const char *fmt0 = fmt; // for checking beginning of output
	fmt              = append(fmt, "```c_sharp\n");
	doc += 10;
	if (*doc == '(') {
		if (doc[1] == 'l') doc += 17;
	}
	while (*doc != '`' || doc[1] != '`' || doc[2] != '`') *fmt++ = *doc++;
	doc += 3;
	while (*--fmt <= ' ') {}
	fmt = append(fmt + 1, "```\n\n");

	if (doc >= docEnd) return fmt - 1;
	char kind = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '\\':
				if (doc[1] != '.' && doc[1] != '(' && doc[1] != ')' && doc[1] != '-') *fmt++ = '\\';
				*fmt++ = *++doc;
				break;
			case '`':
				if (doc[1] == '`' && doc[2] == '`') {
					doc += 4;
					fmt = append(fmt, "```c_sharp");
					while (*doc != '\n') doc++;
					while (*doc != '`' || doc[1] != '`' || doc[2] != '`') *fmt++ = *doc++;
					doc += 2;
					fmt = append(fmt - 1, "```");
				} else {
					*fmt++ = '`';
					while (*++doc != '`') *fmt++ = *doc;
					*fmt++ = '`';
				}
				break;
			case ':':
				if (doc[1] == '\n') {
					in *fmtTmp = (in *) fmt - 1;
					while ((in *) fmt0 < fmtTmp && ' ' < *fmtTmp) fmtTmp--;
					if ('A' <= *++fmtTmp && *fmtTmp <= 'Z' && (fmtTmp == (in *) fmt0 || fmtTmp[-1] <= ' ')) {
						for (int m = fmt - (char *) --fmtTmp; m; m--) fmtTmp[m + 2] = fmtTmp[m];
						*++fmtTmp = '*';
						*++fmtTmp = '*';
						fmt       = append(fmt + 2, ":**");
						if (doc[2] == '\n') doc++;
						break;
					}
				}
				*fmt++ = ':';
				break;
			case '@': {
				const in *docTmp = doc + 1;
				while ('a' <= *docTmp && *docTmp <= 'z') docTmp++;
				if (*docTmp != ' ') {
					*fmt++ = '@';
					break;
				} else if (docTmp - doc == 2) {
					*fmt++ = '`';
					doc += 2;
					while (*++doc > 47) *fmt++ = *doc == '\\' ? *++doc : *doc;
					*fmt++ = '`';
					*fmt++ = *doc;
					break;
				}
				doc++;
				if (*doc == 't' && doc[1] == 'p') doc++;
				resolveKind(&doc, &fmt, &kind);
				if (kind == 'r' || kind == 'p' || kind == 't') {
					fmt = append(fmt, " - ");
					if (kind != 'r') {
						*fmt++ = '`';
						while (*++doc > 47) *fmt++ = *doc == '\\' ? *++doc : *doc;
						if (fmt[-1] == ',') {
							fmt[-1] = '`';
							*fmt++  = ',';
							doc--;
						} else fmt = append(fmt, "`: ");
					}
				}
			} break;
			case '\n':
				while (fmt[-1] == ' ') fmt--;
				if (fmt[-1] == '\n' && kind) {
					kind = 0;
					break;
				} else if (fmt[-1] == '\n' && fmt[-2] == '\n') break;
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
