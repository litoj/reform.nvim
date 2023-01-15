char *append(char *dst, const char *str) {
	while (*str) *dst++ = *str++;
	return dst;
}

int alike(const char *str, const char *cmp) {
	int j = 0;
	while (cmp[j] == str[j] && cmp[j] && str[j]) j++;
	return cmp[j] == '\0' ? j : 0;
}

char *resolveKind(const char *doc, char *fmt, int *docPos, char *kind) {
	int i = *docPos;
	if (*kind != doc[i]) {
		if (!*kind && doc[i] != 'b') *fmt++ = '\n';
		const char *section;
		switch (*kind = doc[i++]) {
			case 'p':
				section = "**Parameters:**\n";
				break;
			case 'r':
				section = "**Returns:**\n";
				break;
			case 's':
				section = "**See:**\n";
				break;
			case 'b': // '@brief'
				*kind = 0;
				section = "";
				break;
			default: // '@other' -> '**Other**:\n'
				*fmt++ = '*';
				*fmt++ = '*';
				*fmt++ = doc[i - 1] - 32; // -32 = 'a' -> 'A'
				while (doc[i] != ' ') *fmt++ = doc[i++];
				section = "**:\n";
				break;
		}
		fmt = append(fmt, section);
	}
	while (doc[i] != ' ') i++;
	*docPos = i;
	return fmt;
}
