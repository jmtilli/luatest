#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>

struct HttpCtx {
  int i;
};

int http2_req(lua_State *lua){
	const char * addr = luaL_checkstring(lua, 1);
	luaL_checktype(lua, 2, LUA_TTABLE);
	int last = lua_toboolean(lua, 3);

	printf("Connecting to %s\n", addr);
	printf("Last is %d\n", last);

	lua_pushvalue(lua, 2);
	lua_pushnil(lua);
	while (lua_next(lua, -2))
	{
		lua_pushvalue(lua, -2);
		const char *key = lua_tostring(lua, -1);
		const char *val = lua_tostring(lua, -2);
		printf("%s => %s\n", key, val);
		lua_pop(lua, 2);
	}
	lua_pop(lua, 1);

	struct HttpCtx *ctx = malloc(sizeof(*ctx));

	ctx->i = 0;

	lua_pushlightuserdata(lua, ctx);
	return 1;
}

int http2_headers(lua_State *lua) {
	struct HttpCtx *ctx = lua_touserdata(lua, 1);

	lua_newtable(lua);
	int top = lua_gettop(lua);

	lua_pushlstring(lua, "foo", 3);
	lua_pushlstring(lua, "bar", 3);
	lua_settable(lua, top);

	lua_pushlstring(lua, "bar", 3);
	lua_pushlstring(lua, "foo", 3);
	lua_settable(lua, top);

	return 1;
}

int http2_datablk(lua_State *lua) {
	struct HttpCtx *ctx = lua_touserdata(lua, 1);

	lua_newtable(lua);
	int top = lua_gettop(lua);

	lua_pushlstring(lua, "last", 4);
	lua_pushboolean(lua, (ctx->i == 0) ? 0 : 1);
	lua_settable(lua, top);

	lua_pushlstring(lua, "data", 4);
	if (ctx->i == 0)
	{
		lua_pushlstring(lua, "foo", 3);
	}
	else if (ctx->i == 1)
	{
		lua_pushlstring(lua, "FOO", 3);
	}
	else
	{
		lua_pushlstring(lua, "", 0);
	}
	lua_settable(lua, top);

	ctx->i++;

	return 1;
}

int http2_free(lua_State *lua){
	struct HttpCtx *ctx = lua_touserdata(lua, 1);
	free(ctx);
	return 1;
}

int luaopen_http2(lua_State *lua)
{
	static const luaL_Reg http2_lib[] = {
		{"req", http2_req},
		{"headers", http2_headers},
		{"datablk", http2_datablk},
		{"free", http2_free},
		{NULL, NULL}};

	luaL_newlib(lua, http2_lib);
	return 1;
}

int main(int argc, char **argv){
	lua_State *lua = luaL_newstate();
	luaL_openlibs(lua);

        luaL_requiref(lua, "Http2", luaopen_http2, 1);

	luaL_dostring(lua, "loadfile(\"test.lua\")()");
	luaL_dostring(lua, "if true then");
	luaL_dostring(lua, "print(\"test successful\")");
	luaL_dostring(lua, "end");

	lua_close(lua);

	return 0;
}
