#include "utils.h"

/**
 * @brief Format java code from jdtls markdown docs
 *
 * @param doc original markdown
 * @param fmt buffer for formatted documentation
 * @param docStart ref to pos of `doc` parsing
 * @param type '>' or ' '
 * @return ptr to current `fmt` position
 */
static char *java_code_fmt(const char *doc, char *fmt, int *docPos, char type) {
	int i = *docPos;
	fmt = append(fmt - (fmt[-2] == '\n'), "```java\n");
	int skip = 1;
	while (doc[i + skip] == ' ') skip++; // strip indent to keep ours' consistent
	while (1) {
		*fmt++ = ' '; // add some indent for easier code distinction
		i += skip;
		while (doc[i] != '\n') *fmt++ = doc[i++];
		if (doc[i + 1] != type) break;
		else *fmt++ = doc[i++];
	}
	*docPos = i - 1;
	return append(fmt, "```");
}

char *java_fmt(const char *doc, char *fmt, int len) {
	int i = -1;
	char kind = 0; // type of currently processed list (p = parameter...)
	if (!alike(doc, "```java")) {
		int j = 0;
		while (doc[j] != '\n') j++;
		if (doc[j + 1] != '\n') {
			fmt = append(fmt, "```java\n");
			// fix TS highlighting (recognize method/class with 'x<y>' `type`)
			// this cannot fix method identifier highlight, only `type` and `parameter`
			int j = 2, cont = 0;
			while (doc[j] && doc[j] != '\n') {
				switch (doc[j++]) {
					case '(':                        // will get here only if '<' was found
						fmt = append(fmt, "default "); // any method keyword for TS to recognize method
						j = len;
						break;
					case '<':
						cont = 1;
						break;
					case ' ':
						if (!cont) j = len;
						break;
				}
			}
			if (doc[j - 1] == '>') fmt = append(fmt, "class "); // fix TS class `type` highlight
			i = 0;
			while (doc[i] && doc[i] != '\n') *fmt++ = doc[i++];
			fmt = append(fmt, "```\n\n");
		}
	}
	while (++i < len) {
		switch (doc[i]) {
			case '>': // code block '>' delimited: '> code'
				if (fmt[-1] == '\n' && fmt[-2] == '\n') fmt = java_code_fmt(doc, fmt, &i, '>');
				else *fmt++ = '>';
				break;
			case '\n': {
				int j = 1;
				while (doc[i + j] == ' ') j++;
				// deal with non-lists (probably italics or bold markers)
				if (!alike(doc + i + j, "*  ")) {
					// code block 4-space delimited: '    code'
					if (j == 5 && doc[i + j] != '\n') fmt = java_code_fmt(doc, fmt, &i, ' ');
					else if (doc[i + j] == '\n' && j > 1) i += j - 1;
					else *fmt++ = '\n';
					break;
				}
				// change list marker from '*' to '-' and halve the indentation (1 + 2 spaces)
				if (fmt[-1] != '\n') *fmt++ = '\n';
				i += j + 2; // skip all indentation + '*  '
				if (j == 2) {
					// sections are 1.st level indent as lists
					if (doc[i + 1] == '*' && doc[i + 2] == '*') {
						if (!kind) *fmt++ = '\n'; // separate start of first found section
						kind = doc[i + 3] + 32;   // record current section type
					} else fmt = append(fmt, " - ");
				} else {         // add half the identation (we use 2, java 4)
					j = j / 2 - 2; // -2: indent/list depth <<1; list indent >= 1
					while (--j > 0) *fmt++ = ' ';
					fmt = append(fmt, " - ");
					// params use format: '     *  **param** desc'
					if (doc[i + 1] == '*' && doc[i + 2] == '*' && kind == 'p') {
						i += 3;
						*fmt++ = '`'; // format into our ' - `param`: desc'
						while (doc[i] != '*') *fmt++ = doc[i++];
						i++;
						*fmt++ = '`';
						*fmt++ = ':';
					}
				}
			} break;
			default:
				*fmt++ = doc[i];
		}
	}
	return fmt;
}
