#include "utils.h"

char *cpp_fmt(const in *doc, char *fmt, int len) {
	const in *docEnd = doc + len;
	if (alike(doc + len - 3, "```") > 0) { // move the end (code declaration) to the beginning
		docEnd                    = doc + len - 6;
		const in *docEndPractical = doc + len - 4; // strip also the newline from \n```$
		while (!alike(docEnd - 3, "\n```")) docEnd--;
		if (docEnd[1] == '\n') { // ```\nsome simple text``` - no detailed docs, only signature
			doc = docEnd + 2;
			while (doc < docEndPractical) *fmt++ = *doc++;
			return fmt;
		}
		docEnd -= 3; // end before ```
		fmt               = append(fmt, "```cpp");
		const in *docCode = docEnd + 3;
		while (*docCode != '\n') docCode++;
		while (docCode < docEndPractical) *fmt++ = *docCode++;
		while (fmt[-1] == '\n') fmt--;
		fmt = append(fmt, ";\n```\n"); // '\n```' -> ';```' to fix syntax highlighting
		if (alike(doc, "###") > 0) {   // strip type defs - already in code block
			char match = 1;
			// array of possible prefix matches
			const char *prefixes[] = {"- `", "---", "Value =", "→", "Param", "Type:"};
			const int n            = sizeof(prefixes) / sizeof(prefixes[0]);
			while (match) {
				while (*doc++ != '\n' || *doc == '\n') {}
				match = 0;
				for (int i = 0; i < n; i++)
					if (alike(doc, prefixes[i]) > 0) {
						match = 1;
						break;
					}
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
					fmt = append(fmt, "```");
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
