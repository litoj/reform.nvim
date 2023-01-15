#include "cpp.h"
#include "java.h"
#include "lua.h"
#include "utils.h"
#include <lauxlib.h>
#include <lua.h>
#include <stdlib.h>

static int l_fmt(lua_State *L) {
	if (lua_gettop(L) != 2) return 0;
	size_t len;
	const char *doc = luaL_checklstring(L, 1, &len);
	const char *ft = luaL_checkstring(L, 2);
	while (*doc == '\n' || *doc == ' ') {
		len--;
		doc++;
	}
	struct {
		char *ft;
		char *(*parser)(const char *, char *, int);
	} avail[4] = {{"lua", lua_fmt}, {"cpp", cpp_fmt}, {"c", cpp_fmt}, {"java", java_fmt}};
	for (int i = 0; i < 4; i++) {
		if (ft[alike(ft, avail[i].ft)] == '\0') {
			char *fmt = (char *) malloc(len + 50);
			char *end = avail[i].parser(doc, fmt, len);
			while (*end && *--end == '\n') {}
			*++end = '\0';
			lua_pushstring(L, realloc(fmt, (end - fmt + 1) * sizeof(char)));
			return 1;
		}
	}
	lua_pushstring(L, doc);
	return 1;
}

int luaopen_reform_docfmt(lua_State *L) {
	// lua_newtable(L);
	lua_pushcfunction(L, l_fmt);
	// lua_setfield(L, -2, "fmt");
	return 1;
}
