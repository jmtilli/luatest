.PHONY: all

all: luatest test.luac

luatest: luatest.c
	cc -g -o luatest -I/usr/include/lua5.3 luatest.c /usr/lib/x86_64-linux-gnu/liblua5.3.a -lm -ldl

test.luac: test.lua
	luac5.3 -o test.luac test.lua
