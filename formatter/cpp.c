#include "utils.h"

char *cpp_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	if (alike(doc + len - 3, "```") > 0) { // move the end (code declaration) to the beginning
		docEnd = doc + len - 6;
		while (*docEnd != '`' || docEnd[-1] != '`' || docEnd[-2] != '`') docEnd--;
		if (docEnd[1] == '\n') { // ```\nsome simple text``` - no detailed docs, only signature
			doc = docEnd + 2;
			while (*doc != '`' || *++doc != '`' || *++doc != '`') *fmt++ = *doc++;
			return fmt;
		}
		docEnd -= 2; // end before ```
		fmt               = append(fmt, "```cpp");
		const in *docCode = docEnd;
		while (*docCode != '\n') docCode++;
		fmt = append(fmt, ";\n```\n"); // '\n```' -> ';```' to fix syntax highlighting
		if (alike(doc, "###") > 0) {   // strip type defs - already in code block
			while (*++doc != '\n' || *++doc == '-' || *doc == '\n') {}
			if (alike(doc, "Param") > 0) {
				while (*++doc != '\n') {}
				while (*++doc != '\n' || *++doc == '-') {}
			} else if (alike(doc, "Type") > 0) {
				while (*++doc != '\n') {}
				doc++;
			}
			*fmt++ = '\n';
		}
	}
	doc--;
	char kind = 0;
	while (++doc < docEnd) {
		switch (*doc) {
			case '\\':
				*fmt++ = '\\';
				*fmt++ = *++doc;
				break;
			case '`':
				if (doc[1] == '`' && doc[2] == '`') {
					doc += 2;
					fmt = append(fmt, "```cpp");
					while (*doc != '\n') doc++;
					while (*doc != '`' || *++doc != '`' || *++doc != '`') *fmt++ = *doc++;
					fmt = append(fmt - 1, "```");
				} else {
					*fmt++ = '`';
					while (*++doc != '`') *fmt++ = *doc;
					*fmt++ = '`';
				}
				break;
			case 226: // →
				if (!alike(doc, "→")) {
					*fmt++ = (char) *doc;
					break;
				}
				doc += 5; // scrap this line
				while (*doc != '\n' || doc[1] != '\n') doc++;
				doc++;
				break;
			case '%':
				if (doc[-1] != ' ') *fmt++ = '%';
				else {
					*fmt++ = '*';
					while (*++doc > ' ') *fmt++ = *doc;
					doc--;
					*fmt++ = '*';
				}
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
				if (alike(doc + 1, "---") > 0) doc += 4;
				if (fmt[-1] == '\n' && kind) {
					kind = 0;
					break;
				}
			default: *fmt++ = *doc;
		}
	}
	return fmt;
}
