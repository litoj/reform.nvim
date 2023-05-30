#include "utils.h"

/**
 * @brief Format java code from jdtls markdown docs
 *
 * @param docPtr ptr to current pos in source docs
 * @param fmtPtr ptr to buffer for formatted docs
 * @param type '>' or ' '
 */
static void java_code_fmt(const char** docPtr, char** fmtPtr, char type) {
	const char* doc = *docPtr;
	char* fmt       = append(*fmtPtr - ((*fmtPtr)[-2] == '\n'), "```java\n");
	int skip        = 1;
	while (doc[skip] == ' ') skip++; // strip indent to keep ours' consistent
	while (1) {
		*fmt++ = ' '; // add some indent for easier code distinction
		doc += skip;
		while (*doc != '\n') *fmt++ = *doc++;
		if (doc[1] != type) break;
		else *fmt++ = *doc++;
	}
	*docPtr = doc - 1;
	*fmtPtr = append(fmt, "```");
}

char* java_fmt(const char* doc, char* fmt, int len) {
	const char* docEnd = doc + len;
	if (!alike(doc, "```java")) {
		const char* docTmp = doc;
		while (*docTmp != '\n') docTmp++;
		if (docTmp[1] != '\n') {
			fmt = append(fmt, "```java\n");
			// fix TS highlighting (recognize method/class with 'x<y>' `type`)
			// this cannot fix method identifier highlight, only `type` and `parameter`
			int cont           = 0;
			const char* docTmp = doc + 2;
			while (*docTmp > '\n' && cont >= 0) {
				switch (*docTmp++) {
					case '(':                         // will get here only if '<' was found
						fmt  = append(fmt, "default "); // any method keyword for TS to recognize method
						cont = -1;
						break;
					case '<':
						cont = 1;
						break;
					case ' ':
						if (!cont) cont = -1;
						break;
				}
			}
			if (docTmp[-1] == '>') fmt = append(fmt, "class "); // fix TS class `type` highlight
			while (*doc > '\n') *fmt++ = *doc++;
			fmt = append(fmt, "```\n");
		}
	}
	doc--;
	char kind = 0; // type of currently processed list (p = parameter...)
	while (++doc < docEnd) {
		switch (*doc) {
			case '>': // code block '>' delimited: '> code'
				if (fmt[-1] == '\n' && fmt[-2] == '\n') java_code_fmt(&doc, &fmt, '>');
				else *fmt++ = '>';
				break;
			case '\n': {
				int i = 1;
				while (doc[i] == ' ') i++;
				// deal with non-lists (probably italics or bold markers)
				if (!alike(doc + i, "*  ")) {
					// code block 4-space delimited: '    code'
					if (i == 5 && doc[i] >= ' ') java_code_fmt(&doc, &fmt, ' ');
					else if (doc[i] == '\n' && i > 1) doc += i - 1;
					else *fmt++ = '\n';
					break;
				}
				// change list marker from '*' to '-' and halve the indentation (1 + 2 spaces)
				if (fmt[-1] != '\n') *fmt++ = '\n';
				doc += i + 2; // skip all indentation + '*  '
				if (i == 2) {
					// sections are 1.st level indent as lists
					if (doc[1] == '*' && doc[2] == '*') {
						if (!kind) *fmt++ = '\n'; // separate start of first found section
						kind = doc[3] + 32;       // record current section type
					} else fmt = append(fmt, " - ");
				} else {         // add half the identation (we use 2, java 4)
					i = i / 2 - 2; // -2: indent/list depth <<1; list indent >= 1
					while (--i > 0) *fmt++ = ' ';
					fmt = append(fmt, " - ");
					// params use format: '     *  **param** desc'
					if (doc[1] == '*' && doc[2] == '*' && kind == 'p') {
						doc += 3;
						*fmt++ = '`'; // format into our ' - `param`: desc'
						while (*doc != '*') *fmt++ = *doc++;
						doc++;
						*fmt++ = '`';
						*fmt++ = ':';
					}
				}
			} break;
			default:
				*fmt++ = *doc;
		}
	}
	return fmt;
}
