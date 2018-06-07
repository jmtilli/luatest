#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <sys/time.h>

struct HttpCtx {
  int i;
};

int http2_req(lua_State *lua){
        const char * addr = luaL_checkstring(lua, 1);
        luaL_checktype(lua, 2, LUA_TTABLE);
        int last = lua_toboolean(lua, 3);

        if (last)
        {
            printf("Connecting to %s\n", addr);
            printf("Last is %d\n", last);
        }

        lua_pushvalue(lua, 2);
        lua_pushnil(lua);
        while (lua_next(lua, -2))
        {
                lua_pushvalue(lua, -2);
                const char *key = lua_tostring(lua, -1);
                const char *val = lua_tostring(lua, -2);
                if (last)
                {
                        printf("%s => %s\n", key, val);
                }
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

void luacorotbl(lua_State *lua, int i)
{
  int b;

  lua_getglobal(lua, "cotbl");
  lua_getglobal(lua, "coroutine");
  lua_getfield(lua, -1, "create");
  lua_getglobal(lua, "coro");
  lua_call(lua, 1, 1); // stack after this: cotbl, coroutine, result
  lua_pushnumber(lua, i); // stack after: cotbl, coroutine, result, i
  lua_pushvalue(lua, -2); // stack after: cotbl, coroutine, r, i, r
  lua_settable(lua, -5); // stack after: cotbl, coroutine, r
  lua_pop(lua, 3);

  lua_getglobal(lua, "cotbl");
  lua_pushnumber(lua, i); // stack after: cotbl, i
  lua_gettable(lua, -2); // stack after: cotbl, cotbl[i]
  lua_getglobal(lua, "coroutine");
  lua_getfield(lua, -1, "resume"); // after: cotbl, cotbl[i], coroutine, coroutine.resume
  lua_pushvalue(lua, -3); // after: cotbl, cotbl[i], coroutine, coroutine.resume, cotbl[i]
  lua_call(lua, 1, 2); // after: cotbl, cotbl[i], coroutine, ok, args
  if (!lua_isboolean(lua, -2) || !lua_toboolean(lua, -2))
  {
    printf("not ok\n");
  }
  else
  {
    //printf("ok\n");
  }
  lua_pop(lua, 5);

#if 0
  lua_pushnumber(lua, i);
  lua_setglobal(lua, "idx");
  luaL_dostring(lua, "cotbl[idx] = coroutine.create(coro)");
  luaL_dostring(lua, "ok,args = coroutine.resume(cotbl[idx])");
#endif

  if (lua_gettop(lua) != 0)
  {
    printf("top %d\n", lua_gettop(lua));
    exit(1);
  }
}

void luacoro(lua_State *lua)
{
  int it = 0;
  int b;
  lua_getglobal(lua, "coroutine");
  lua_getfield(lua, -1, "create");
  lua_getglobal(lua, "coro");
  lua_call(lua, 1, 1);
  lua_setglobal(lua, "co");
  lua_pop(lua, 1);
  do {
    lua_getglobal(lua, "coroutine");
    lua_getfield(lua, -1, "resume");
    lua_getglobal(lua, "co");
    lua_call(lua, 1, 2);
    b = lua_toboolean(lua, -2);
    lua_pop(lua, 3);
    it++;
  } while (b);
  if (lua_gettop(lua) != 0)
  {
    printf("top %d\n", lua_gettop(lua));
    exit(1);
  }
  if (it != 5)
  {
    printf("it %d\n", it);
    exit(1);
  }
}

void luacoroperf(lua_State *lua)
{
  int i;
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  for (i = 0; i < 10*1000; i++)
  {
    luacoro(lua);
  }
  gettimeofday(&tv2, NULL);
  printf("Time %g us / iter\n",
         (tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1e6)/10e3*1e6);
}

void luaperf(lua_State *lua)
{
  int i;
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  for (i = 0; i < 100*1000; i++)
  {
    lua_getglobal(lua, "f");
    lua_call(lua, 0, 0);
    //luaL_dostring(lua, "f()");
  }
  gettimeofday(&tv2, NULL);
  printf("Time %g us / iter\n",
         (tv2.tv_sec-tv1.tv_sec + (tv2.tv_usec-tv1.tv_usec)/1e6)/100e3*1e6);
}

int main(int argc, char **argv){
        lua_State *lua = luaL_newstate();
        luaL_openlibs(lua);
        char *line = NULL;
        ssize_t nread;
        size_t len = 0;
        int i;
        int thresh1 = 0, thresh2 = 0;

        //luaL_requiref(lua, "Http2", luaopen_http2, 1);
        //lua_pop(lua, 1);

        lua_pushcfunction(lua, luaopen_http2);
        lua_pushstring(lua, "Http2");
        lua_call(lua, 1, 1);
        lua_pushvalue(lua, -1);
        lua_setglobal(lua, "Http2");
        lua_pop(lua, 1);
        printf("load top: %d\n", lua_gettop(lua));

        luaL_dostring(lua, "loadfile(\"test.luac\")()");
#if 0
        luaL_dostring(lua, "if false then");
        luaL_dostring(lua, "print(\"test successful\")");
        luaL_dostring(lua, "end");
#endif

        printf("Coroutine test\n");

        luacoro(lua);

        printf("Perf test\n");

        luaperf(lua);

        printf("Coroutine perf test\n");

        luacoroperf(lua);

        printf("Coroutine memory test\n");
        luaL_dostring(lua, "cotbl = {}");
	lua_gc(lua, LUA_GCSETPAUSE, 200);
	lua_gc(lua, LUA_GCSETSTEPMUL, 100);

        for (i = 0; i < 100*1000; i++)
        {
                struct timeval tv1, tv2;
                double secs;
                gettimeofday(&tv1, NULL);
                luacorotbl(lua, i);
		lua_gc(lua, LUA_GCSTEP, 0);
                gettimeofday(&tv2, NULL);
                secs = (tv2.tv_sec-tv1.tv_sec)+(tv2.tv_usec-tv1.tv_usec)/1e6;
                if (secs > 1e-3 && !thresh1 && i < 10000)
                {
                        printf("1: iter %d secs %g\n", i, secs);
                        thresh1 = 1;
                }
                if (secs > 1e-2 && !thresh2 && i < 100000)
                {
                        printf("2: iter %d secs %g\n", i, secs);
                        thresh2 = 1;
                }
        }

        printf("Entering command loop mode, press ^D to end\n");

        lua_settop(lua, 0);

        while ((nread = getline(&line, &len, stdin)) != -1) {
            if (strlen(line) != nread)
            {
                printf("Seen NUL character\n");
            }
            luaL_dostring(lua, line);
            lua_settop(lua, 0);
        }

        free(line);

        lua_close(lua);

        return 0;
}
