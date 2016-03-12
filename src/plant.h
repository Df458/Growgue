#ifndef PLANT_H
#define PLANT_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct map map;

typedef struct plant
{
    int color;
    int display;
    char* name;

    int growth_time;
    float water_need;
    float nutrient_need;
    float mineral_need;

    int x;
    int y;
    map* map_ref;

    bool can_harvest;
    bool dead;

    char* item_id;
    int item_count;

    lua_State* script_state;
}
plant;

plant* create_plant(const char* file, int x, int y, map* ref);
void kill_plant(plant* pl);
void update_plant(plant* pl, int delta);

void insert_plant_into_lua(lua_State* state, plant* it);
int lua_plant_index(lua_State* state);
int lua_plant_newindex(lua_State* state);

static const luaL_Reg plant_funcs[] =
{
    {0, 0}
};

static const luaL_Reg plant_meta[] =
{
    {"__index", lua_plant_index},
    {"__newindex", lua_plant_newindex},
    {0, 0}
};

#endif
