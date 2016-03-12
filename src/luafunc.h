#ifndef LUA_FUNC_H
#define LUA_FUNC_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef PDCURSES
#include <xcurses/curses.h>
#else
#include <curses.h>
#endif

bool callback(const char* name, lua_State* state);

int lua_log_message(lua_State* state);
int lua_ask_question(lua_State* state);

static const luaL_Reg global_funcs[] =
{
    {"log", lua_log_message},
    {"ask", lua_ask_question},
    {0, 0}
};

#endif
