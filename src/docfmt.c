#include "cpp.h"
#include "java.h"
#include "lua.h"
#include "utils.h"
#include <lauxlib.h>
#include <lua.h>
#include <stdlib.h>

static int l_fmt(lua_State* L) {
	if (lua_gettop(L) != 2) return 0;
	size_t len;
	const char* doc = luaL_checklstring(L, 1, &len);
	const char* ft  = luaL_checkstring(L, 2);
	while (*doc == '\n' || *doc == ' ') {
		len--;
		doc++;
	}
	struct {
		const char* ft;
		char* (*parser)(const char*, char*, int);
	} avail[4] = {{"lua", lua_fmt}, {"cpp", cpp_fmt}, {"c", cpp_fmt}, {"java", java_fmt}};
	for (int i = 0; i < 4; i++) {
		int match = alike(ft, avail[i].ft);
		if (match > 0 && !ft[match]) {
			char* fmt = (char*) malloc(len + 50);
			char* end = avail[i].parser(doc, fmt, len);

			if (end > fmt)
				while (*--end == '\n') {}
			*++end          = '\n';
			*++end          = '\0';
			fmt             = (char*) realloc(fmt, end - fmt + 1);

			const char* ptr = fmt;
			lua_newtable(L);
			for (int j = 1; *fmt; j++, ptr = fmt) {
				while (*fmt != '\n') fmt++;
				*fmt++ = '\0';
				lua_pushstring(L, ptr);
				lua_rawseti(L, -2, j);
			}
			return 1;
		}
	}

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
}
