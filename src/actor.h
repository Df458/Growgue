#ifndef ACTOR_H
#define ACTOR_H
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

typedef struct actor
{
    int color;
    int display;

    bool to_kill;
    bool angry;

    int x;
    int y;
    int aggro;
    int hp_max;
    int hp;
    int str;
    int def;
    int xp;

    char* name;

    char** drops;
    int* drop_rarities;
    int* drop_counts;
    int drop_count;

    lua_State* script_state;
}
actor;

typedef struct map map;

actor* create_actor(const char* file);
void init_actor(actor* act);
void update_actor(actor* act, map* cmap);
void kill_actor(actor* act, bool force);
int lua_kill_actor(lua_State* state);
int damage_actor(actor* act, int damage);
void drop_loot(actor* act, map* cmap);

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
