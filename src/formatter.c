#include "bash.h"
#include "cpp.h"
#include "csharp.h"
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

// clang-format off
#define p(fmt) { #fmt, fmt##_fmt, #fmt"\n"}
// clang-format on
static struct pair {
	const char *ft;
	char *(*formatter)(const in *, char *, int);
	const char *label;
} fmts[] = {
  p(lua),
  {"cpp", cpp_fmt, NULL},
  {"c", cpp_fmt, NULL},
  {"cs", csharp_fmt, "csharp\n"},
  p(java),
  {"bash", bash_fmt, " man\n"},
  {"sh", bash_fmt, " man\n"},
  {"javascript", typescript_fmt, "typescript\n"},
  p(typescript)};
static const int fmtsN = sizeof(fmts) / sizeof(struct pair);

#ifdef DEBUG
int main(int argc, char *argv[]) {
	in *doc;
	if (argc < 2) {
		printf("usage: ./main.out FileType TestString\n   or: ./main.out FileType < test.txt\n");
		return 1;
	}
	size_t len = 0;
	char raw   = alike((in *) argv[argc - 1], "-r") > 0;
	if (argc < 3 || raw) {
		size_t alloc = 32768;
		doc          = (in *) malloc(sizeof(in) * alloc);
		size_t size  = 0;
		while ((size = fread(doc + len, 1, alloc - len, stdin)) > 0) {
			if ((len += size) >= alloc - 1) doc = (in *) realloc(doc, sizeof(in) * (alloc *= 2));
		}
		doc[len] = '\0';
	} else {
		doc = (in *) argv[2];
		while (doc[++len]) {}
	}
	if (doc[len - 1] == '\n') doc[--len] = 0;
	const char *ft = argv[1];

#else

static int l_fmt(lua_State *L) {
	if (lua_gettop(L) != 2) return 0;
	size_t len;
	const in *doc  = (in *) luaL_checklstring(L, 1, &len);
	const char *ft = luaL_checkstring(L, 2);
	while (*doc == '\n' || *doc == ' ') {
		len--;
		doc++;
	}
#endif

	for (int i = 0; i < fmtsN; i++) {
		int match = alike((in *) ft, fmts[i].ft);
		if (match > 0 && !ft[match]) {
			if (fmts[i].label && alike(doc + 3, fmts[i].label) <= 0) continue; // filter some snippets
			char *fmt = (char *) malloc((len + 50) * sizeof(char));
			char *end = fmts[i].formatter(doc, fmt, len);

			if (end > fmt)
				while ((in) * --end <= (in) ' ') {}
			*++end = '\n';
			*++end = '\0';
#ifdef DEBUG
			// printf("\033[34morigin\033[91m: \033[32mlen\033[31m=\033[95m%ld\033[0m\n%s\n", len, doc);
			len = end - fmt + 1;
			if (!raw) printf("\033[32mlen\033[31m=\033[95m%ld\n\033[91m------------\033[0m\n", len);
#endif
			char *ptr = fmt = (char *) realloc(fmt, (end - fmt + 1) * sizeof(char));
			char *start     = fmt;
#ifndef DEBUG
			lua_newtable(L);
#endif
			if (*fmt == '\n') fmt++;
			while (*(ptr = fmt) <= ' ') { // remove empty lines
				while (*++fmt == ' ') {}
				if (*fmt != '\n') break;
			}
			fmt = ptr;

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
			free(start);
#ifdef DEBUG
			if (argc < 3 || raw) free(doc);
			return 0;
#else
			return 1;
#endif
		}
	}
#ifdef DEBUG
	if (argc < 3) free(doc);
	printf("\033[91mUnsupported filetype: %s\033[0m\nSupported ft are:\n", ft);
	for (int i = 0; i < fmtsN; i++) {
		printf(" \033[32m%s\033[31m: \033[33m```%s", fmts[i].ft, fmts[i].label);
	}
	return 1;
#else
	char *fmt       = (char *) malloc(len + 1);
	const char *end = (char *) doc + len, *ptr = fmt;
	lua_newtable(L);
	for (int j = 1; (char *) doc < end; j++, ptr = fmt, doc++) {
		while (*doc != '\n' && *doc) *fmt++ = *doc++;
		*fmt++ = '\0';
		lua_pushstring(L, ptr);
		lua_rawseti(L, -2, j);
	}
	return 1;
}

int luaopen_reform_formatter(lua_State *L) {
	lua_pushcfunction(L, l_fmt);
	return 1;
#endif
}
