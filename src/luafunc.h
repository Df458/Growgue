#ifndef LUA_FUNC_H
#define LUA_FUNC_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef PDCURSES
#ifdef Linux
#include <xcurses/curses.h>
#include <xcurses/panel.h>
#else
#include <curses.h>
#include <panel.h>
#endif
#else
#include <curses.h>
#include <panel.h>
#endif

bool callback(const char* name, lua_State* state);

int lua_log_message(lua_State* state);
int lua_ask_question(lua_State* state);
int lua_finish(lua_State* state);
bool game_won();

static const luaL_Reg global_funcs[] =
{
    {"log", lua_log_message},
    {"ask", lua_ask_question},
    {"finish", lua_finish},
    {0, 0}
};

#endif
