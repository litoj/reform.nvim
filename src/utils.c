char* append(char* dst, const char* str) {
	while (*str) *dst++ = *str++;
	return dst;
}

int alike(const char* str, const char* cmp) {
	int j = 0;
	while (cmp[j] && cmp[j] == str[j]) j++;
	return cmp[j] == '\0' ? j : 0;
}

char* resolveKind(const char* doc, char* fmt, int* docPos, char* kind) {
	int i = *docPos;
	if (*kind != doc[i]) {
		if (doc[i + 1] != ' ') {
			if (!*kind && doc[i] != 'b') *fmt++ = '\n';
			const char* section;
			switch (doc[i]) {
				case 'p':
					section = "**Parameters:**\n";
					*kind = doc[i++];
					break;
				case 'r':
					section = "**Returns:**\n";
					*kind = doc[i++];
					break;
				case 's':
					section = "**See:**\n";
					*kind = doc[i++];
					break;
				case 't':
					section = "**Throws:**\n";
					*kind = doc[i++];
					break;
				case 'b': // '@brief'
					section = "";
					*kind = 0;
					i++;
					break;
				default: // '@other' -> '**Other**:\n'
					*fmt++ = '*';
					*fmt++ = '*';
					*fmt++ = *kind = doc[i++] - 32; // -32 = 'a' -> 'A'
					while (doc[i] != ' ') *fmt++ = doc[i++];
					section = "**:\n";
					break;
			}
			fmt = append(fmt, section);
		} else {
			i++;
			*fmt++ = '`';
			while (doc[++i] > ' ') {
				if (doc[i] == ',') fmt = append(fmt, "`,`");
				else if (doc[i] != '\\') *fmt++ = doc[i];
			}
			*fmt++ = '`';
			*fmt++ = doc[i];
		}
	}
	while (doc[i] != ' ') i++;
	*docPos = i;
	return fmt;
}
