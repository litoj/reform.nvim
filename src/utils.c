char* append(char* dst, const char* str) {
	while (*str) *dst++ = *str++;
	return dst;
}

int alike(const char* str, const char* cmp) {
	int j = 0;
	while (cmp[j] && cmp[j] == str[j]) j++;
	return cmp[j] ? -!str[j] : j;
}

char* resolveKind(const char* doc, char* fmt, int* docPos, char* kind) {
	int i = *docPos;
	// we rely on parsers to catch `@a` and `@p` themselves
	if (!*kind && !alike(doc + i, "brief")) *fmt++ = '\n';
	const char* section;
	int skip;
	if ((skip = alike(doc + i, "param"))) section = "**Parameters:**\n";
	else if ((skip = alike(doc + i, "return"))) section = "**Returns:**\n";
	else if ((skip = alike(doc + i, "throw"))) section = "**Throws:**\n";
	else if ((skip = alike(doc + i, "see"))) section = "**See:**\n";
	else if ((skip = alike(doc + i, "ingroup"))) {
		i += skip;
		while (doc[i] >= ' ') i++;
		*docPos = i;
		return fmt;
	} else if ((skip = alike(doc + i, "brief"))) {
		*kind   = 0;
		*docPos = i + skip;
		return fmt;
	} else { // '@other' -> '**Other**:\n'
		*fmt++ = '*';
		*fmt++ = '*';
		*fmt++ = *kind = doc[i++] - 32; // -32 = 'a' -> 'A'
		while (doc[i] != ' ') *fmt++ = doc[i++];
		*docPos = i;
		return append(fmt, "**: ");
	}
	if (*kind != doc[i]) fmt = append(fmt, section);
	*kind = doc[i];
	i += skip;
	while (doc[i] != ' ') i++;
	*docPos = i;
	return fmt;
}
