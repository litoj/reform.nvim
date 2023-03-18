#include "cpp.h"
#include "java.h"
#include "lua.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char* doc;
	if (argc < 2) {
		printf("usage: debug FileType TestString\n   or: debug FileType < file.txt\n");
		return 1;
	}
	if (argc < 3) {
		doc = (char*) malloc(32768);
		scanf("%[32767]s", doc);
	} else doc = argv[2];
	const char* ft = argv[1];
	size_t len = 0;
	while (doc[++len]) {}
	while (*doc == '\n' || *doc == ' ') {
		len--;
		doc++;
	}
	struct {
		const char* ft;
		char* (*parser)(const char*, char*, int);
	} avail[4] = {{"lua", lua_fmt}, {"cpp", cpp_fmt}, {"c", cpp_fmt}, {"java", java_fmt}};
	for (int i = 0; i < 4; i++) {
		if (!ft[alike(ft, avail[i].ft)]) {
			char* fmt = (char*) malloc(len + 50);
			char* end = avail[i].parser(doc, fmt, len);

			if (end > fmt)
				while (*--end == '\n') {}
			*++end = '\n';
			*++end = '\0';
			printf("\033[34morigin\033[91m: \033[32mlen\033[31m=\033[95m%ld\033[0m\n%s\n", len, doc);
			len = end - fmt + 1;
			fmt = (char*) realloc(fmt, len * sizeof(char));
			printf("\033[34mresult\033[91m: \033[32mlen\033[31m=\033[95m%ld\n\033[0m%s\033[91m------------\033[0m\n", len, fmt);

			char* start = fmt;
			const char* ptr = fmt;
			for (int j = 1; *fmt; j++, ptr = fmt) {
				while (*fmt != '\n') fmt++;
				*fmt++ = '\0';
				printf("\033[95m%4ld\033[91m:\033[0m %s\n", ptr - start, ptr);
			}
			free(start);
			if (argc < 3) free(doc);
			return 0;
		}
	}
	printf("\033[91mUnsupported filetype: %s\033[0m\n%s\n", ft, doc);
	if (argc < 3) free(doc);
	return 1;
}
