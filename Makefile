luatest: luatest.c
	cc -o luatest -I/usr/include/lua5.3 luatest.c /usr/lib/x86_64-linux-gnu/liblua5.3.a -lm -ldl
