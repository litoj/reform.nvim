#include "utils.h"

char* cpp_fmt(const char* doc, char* fmt, int len) {
	const char* docEnd = doc + len;
	if (alike(doc + len - 3, "```") > 0) { // move the end (code declaration) to the beginning
		docEnd = doc + len - 6;
		while (*docEnd != '`' || docEnd[-1] != '`' || docEnd[-2] != '`') docEnd--;
		if (docEnd[1] == '\n') {
			docEnd += 2;
			while (*docEnd != '\n') *fmt++ = *docEnd++;
			return fmt;
		}
		docEnd -= 2; // end before ```
		fmt                 = append(fmt, "```cpp");
		const char* docCode = docEnd;
		while (*docCode != '\n') docCode++;
		while (*docCode) *fmt++ = *docCode++;
		fmt[-4] = ';'; // '\n```' -> ';```' to fix syntax highlighting
		*fmt++  = '\n';
		if (alike(doc, "###") > 0) { // strip type defs - already in code block
			while (*++doc != '\n' || *++doc == '-' || *doc == '\n') {}
			if (alike(doc, "Param") > 0 || alike(doc, "Type") > 0) {
				while (*++doc != '\n') {}
				while (*++doc != '\n' || *++doc == '-') {}
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
			case -110:  // →
				fmt -= 2; // → is a 3-byte char
				doc += 3;
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
				int i = 1;
				while (doc[i] >= 'a' && doc[i] <= 'z') i++;
				if (doc[i] != ' ') {
					*fmt++ = '@';
					break;
				} else if (i == 2) {
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
			default:
				*fmt++ = *doc;
		}
	}
	return fmt;
}
