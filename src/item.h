#ifndef ITEM_H
#define ITEM_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
// #include <stdbool.h>

typedef struct item
{
    bool on_ground;
    int x;
    int y;
    int count;

    // Usage
    bool can_equip;
    bool can_use;
    bool can_plant;

    // Stats for equipment
    int slot;
    int str;
    int def;

    char* name;
    char display;
    int color;

    char* orig_path;
    char* plant_id;

    lua_State* script_state;
}
item;

void init_items();
void cleanup_items();
item* create_item(const char* path);
item* clone_item(const item* it);
void destroy_item(item* item);
item* get_item(item** item_list, int item_count, int purpose, bool auto_select);

void insert_item_into_lua(lua_State* state, item* it);
int lua_item_index(lua_State* state);
int lua_item_newindex(lua_State* state);

static const luaL_Reg item_funcs[] =
{
    {0, 0}
};

static const luaL_Reg item_meta[] =
{
    {"__index", lua_item_index},
    {"__newindex", lua_item_newindex},
    {0, 0}
};

#endif
