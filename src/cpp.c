#include "utils.h"

char* cpp_fmt(const char* doc, char* fmt, int len) {
	int i = -1, end = len;
	if (alike(doc + len - 3, "```")) { // move the end (code declaration) to the beginning
		len -= 6;
		while (doc[len] != '`' || doc[len - 1] != '`' || doc[len - 2] != '`') len--;
		if (doc[len + 1] == '\n') {
			len += 2;
			while (doc[len] != '\n') *fmt++ = doc[len++];
			return fmt;
		}
		end = (len -= 2); // end before ```
		fmt = append(fmt, "```cpp");
		while (doc[len] != '\n') len++;
		while (doc[len]) *fmt++ = doc[len++];
		fmt[-4] = ';'; // '\n```' -> ';```' to fix syntax highlighting
		*fmt++  = '\n';
		if (alike(doc, "###")) { // strip type defs - already in code block
			while (doc[++i] != '\n' || doc[++i] == '-' || doc[i] == '\n') {}
			if (alike(doc + i, "Param") || alike(doc + i, "Type")) {
				while (doc[++i] != '\n') {}
				while (doc[++i] != '\n' || doc[++i] == '-') {}
			}
			i--;
			*fmt++ = '\n';
		}
	}
	char kind = 0;
	while (++i < end) {
		switch (doc[i]) {
			case '\\':
				*fmt++ = doc[i++];
				*fmt++ = doc[i];
				break;
			case '`':
				if (doc[i + 1] == '`' && doc[i + 2] == '`') {
					i += 2;
					fmt = append(fmt, "```cpp");
					while (doc[i] != '\n') i++;
					while (doc[i] != '`' || doc[++i] != '`' || doc[++i] != '`') *fmt++ = doc[i++];
					fmt = append(fmt - 1, "```");
				} else {
					*fmt++ = '`';
					while (doc[++i] != '`') *fmt++ = doc[i];
					*fmt++ = '`';
				}
				break;
			case -110:  // →
				fmt -= 2; // → is a 3-byte char
				i += 3;
				while (doc[i] != '\n' || doc[i + 1] != '\n') i++;
				i++;
				break;
			case '%':
				if (doc[i - 1] != ' ') *fmt++ = '%';
				else {
					*fmt++ = '*';
					while (doc[++i] > ' ') *fmt++ = doc[i];
					i--;
					*fmt++ = '*';
				}
				break;
			case '@': {
				int j = 1;
				while (doc[i + j] >= 'a' && doc[i + j] <= 'z') j++;
				if (doc[i + j] != ' ') {
					*fmt++ = '@';
					break;
				} else if (j == 2) {
					*fmt++ = '`';
					i += 2;
					while (doc[++i] > 47) *fmt++ = doc[doc[i] == '\\' ? ++i : i];
					*fmt++ = '`';
					*fmt++ = doc[i];
					break;
				}
				i++;
				if (doc[i] == 't' && doc[i + 1] == 'p') i++;
				fmt = resolveKind(doc, fmt, &i, &kind);
				if (kind == 'r' || kind == 'p' || kind == 't') {
					fmt = append(fmt, " - ");
					if (kind != 'r') {
						*fmt++ = '`';
						while (doc[++i] > 47) *fmt++ = doc[doc[i] == '\\' ? ++i : i];
						if (fmt[-1] == ',') {
							fmt[-1] = '`';
							*fmt++  = ',';
							i--;
						} else fmt = append(fmt, "`: ");
					}
				}
			} break;
			case '\n':
				while (fmt[-1] == ' ') fmt--;
				if (alike(doc + i + 1, "---")) i += 4;
				if (fmt[-1] == '\n' && kind) {
					kind = 0;
					break;
				}
			default:
				*fmt++ = doc[i];
		}
	}
	return fmt;
}
