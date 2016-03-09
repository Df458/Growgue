#ifndef ACTOR_H
#define ACTOR_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdbool.h>

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
    // TODO: Scripts and other stats
    lua_State* script_state;
}
actor;

actor* create_actor(const char* file);
void init_actor(actor* act);
void kill_actor(actor* act);
int lua_kill_actor(lua_State* state);

static const luaL_Reg actor_funcs[] =
{
    {"kill", lua_kill_actor},
    {0, 0}
};

#endif
