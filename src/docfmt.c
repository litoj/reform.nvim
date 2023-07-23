#include "bash.h"
#include "cpp.h"
#include "java.h"
#include "lua.h"
#include "typescript.h"
#include "utils.h"
#ifdef DEBUG
#include <stdio.h>
#else
#include <lauxlib.h>
#include <lua.h>
#endif
#include <stdlib.h>

#define p(fmt)                                                                                     \
	{                                                                                                \
#fmt, fmt##_fmt                                                                                \
	}
static struct pair {
	const char* ft;
	char* (*parser)(const char*, char*, int);
} avail[] = {
  p(lua),
  p(cpp),
  {"c", cpp_fmt},
  p(java),
  p(bash),
  {"sh", bash_fmt},
  {"javascript", typescript_fmt},
  {"typescript", typescript_fmt}};
static int langs = sizeof(avail) / sizeof(struct pair);

#ifdef DEBUG
int main(int argc, char* argv[]) {
	char* doc;
	if (argc < 2) {
		printf("usage: ./main.out FileType TestString\n   or: ./main.out FileType < test.txt\n");
		return 1;
	}
	size_t len = 0;
	if (argc < 3) {
		size_t alloc = 32768;
		doc          = (char*) malloc(sizeof(char) * alloc);
		size_t size  = 0;
		while ((size = fread(doc + len, 1, alloc - len, stdin)) > 0) {
			if (len == alloc) doc = (char*) realloc(doc, sizeof(char) * (alloc *= 2));
			len += size;
		}
		if (len == alloc) doc = (char*) realloc(doc, sizeof(char) * (alloc += 1));
		doc[len] = '\0';
	} else {
		doc = argv[2];
		while (doc[++len]) {}
	}
	const char* ft = argv[1];

#else

static int l_fmt(lua_State* L) {
	if (lua_gettop(L) != 2) return 0;
	size_t len;
	const char* doc = luaL_checklstring(L, 1, &len);
	const char* ft  = luaL_checkstring(L, 2);
	while (*doc == '\n' || *doc == ' ') {
		len--;
		doc++;
	}
#endif

	for (int i = 0; i < langs; i++) {
		int match = alike(ft, avail[i].ft);
		if (match > 0 && !ft[match]) {
			char* fmt = (char*) malloc(len + 50);
			char* end = avail[i].parser(doc, fmt, len);

			if (end > fmt)
				while (*--end == '\n') {}
			*++end = '\n';
			*++end = '\0';
#ifdef DEBUG
			// printf("\033[34morigin\033[91m: \033[32mlen\033[31m=\033[95m%ld\033[0m\n%s\n", len, doc);
			len = end - fmt + 1;
			printf("\033[32mlen\033[31m=\033[95m%ld\n\033[91m------------\033[0m\n", len);
#endif
			fmt             = (char*) realloc(fmt, end - fmt + 1);

			const char* ptr = fmt;
#ifdef DEBUG
			char* start = fmt;
#else
			lua_newtable(L);
#endif
			for (int j = 1; *fmt; j++, ptr = fmt) {
				while (*fmt != '\n') fmt++;
				*fmt++ = '\0';
#ifdef DEBUG
				printf("%s\n", ptr);
#else
				lua_pushstring(L, ptr);
				lua_rawseti(L, -2, j);
#endif
			}
#ifdef DEBUG
			free(start);
			if (argc < 3) free(doc);
			return 0;
#else
			return 1;
#endif
		}
	}
#ifdef DEBUG
	if (argc < 3) free(doc);
	printf("\033[91mUnsupported filetype: %s\033[0m\n%s\n", ft, doc);
	return 0;
#else
	char* fmt       = (char*) malloc(len + 1);
	const char *end = doc + len, *ptr = fmt;
	lua_newtable(L);
	for (int j = 1; doc < end; j++, ptr = fmt, doc++) {
		while (*doc != '\n' && *doc) *fmt++ = *doc++;
		*fmt++ = '\0';
		lua_pushstring(L, ptr);
		lua_rawseti(L, -2, j);
	}
	return 1;
}

int luaopen_reform_docfmt(lua_State* L) {
	lua_pushcfunction(L, l_fmt);
	return 1;
#endif
}
