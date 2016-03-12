#ifndef ACTOR_H
#define ACTOR_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef PDCURSES
#include <xcurses/panel.h>
#else
#include <panel.h>
#endif

typedef struct actor
{
    int color;
    int display;

    bool to_kill;

    int x;
    int y;
    int aggro;
    int hp_max;
    int hp;
    int str;
    int def;

    char* name;

    lua_State* script_state;
}
actor;

typedef struct map map;

actor* create_actor(const char* file);
void init_actor(actor* act);
void update_actor(actor* act, map* cmap);
void kill_actor(actor* act);
int lua_kill_actor(lua_State* state);
int damage_actor(actor* act, int damage);

void insert_actor_into_lua(lua_State* state, actor*);
int lua_actor_index(lua_State* state);
int lua_actor_newindex(lua_State* state);

static const luaL_Reg actor_meta[] =
{
    {"__index", lua_actor_index},
    {"__newindex", lua_actor_newindex},
    {0, 0}
};

static const luaL_Reg actor_funcs[] =
{
    {"kill", lua_kill_actor},
    {0, 0}
};

#endif
