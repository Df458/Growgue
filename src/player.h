#ifndef PLAYER_H
#define PLAYER_H
#include <curses.h>
#include "map.h"

enum item_purpose {
    PURPOSE_NONE = -1,
    PURPOSE_PICKUP,
    PURPOSE_DROP,
    PURPOSE_APPLY,
    PURPOSE_PLANT,
    PURPOSE_EQUIP,
    PURPOSE_COUNT
};

void init_player(WINDOW* mapw, WINDOW* stats, WINDOW* hp, map* start_map);
void update_player();
void draw_player(int x, int y);
void cleanup_player();
void player_get_position(int* px, int* py);
void player_move(int direction);
void player_act();
int damage_player(int damage);
bool is_dead();
void add_item(item* item);
void remove_item(item* item, int quantity);

void insert_player_into_lua(lua_State* state);
int lua_player_index(lua_State* state);
int lua_player_newindex(lua_State* state);

static const luaL_Reg player_funcs[] =
{
    {0, 0}
};

static const luaL_Reg player_meta[] =
{
    {"__index", lua_player_index},
    {"__newindex", lua_player_newindex},
    {0, 0}
};

#endif
