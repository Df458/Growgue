#ifndef LUA_FUNC_H
#define LUA_FUNC_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int lua_log_message(lua_State* state);

static const luaL_Reg global_funcs[] =
{
    {"log", lua_log_message},
    {0, 0}
};

#endif
