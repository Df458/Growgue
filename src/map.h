#ifndef MAP_H
#define MAP_H
#ifdef PDCURSES
#include <xcurses/curses.h>
#else
// #include <curses.h>
#endif
#include "tile.h"

enum map_gen {
    GEN_BLANK = -1,
    GEN_WALK,
    GEN_TYPE_COUNT
};

typedef struct map
{
    tile*   tiles;
    actor** actors;
    int actor_count;
    int width;
    int height;

    int ds_x;
    int ds_y;

    int us_x;
    int us_y;
}
map;

void init_map(WINDOW* map);
map* create_map(int width, int height, int gen_type, bool has_up, bool has_down);
void update_map(int delta, map* to_update);
void destroy_map(map* to_destroy);
void draw_map(int x, int y, map* to_draw);
int can_move(int x, int y, map* to_move);
bool spawn_actor(int x, int y, const char* file, map* to_spawn);
bool spawn_item(int x, int y, const char* file, map* to_spawn);
bool spawn_plant(int x, int y, const char* file, map* to_spawn);
bool can_plant(int x, int y, map* cmap, bool explain);
void set_weights(int x, int y, map* to_update);
bool step_towards_player(actor* act, map* cmap);
actor* get_actor_at(int x, int y, map* cmap);
void till(int x, int y, map* cmap);
int get_cost(int x, int y, map* cmap);
void place_item(int x, int y, item* it, map* cmap);
void take_item(int x, int y, item* it, map* cmap);
item* harvest_plant(int x, int y, map* cmap);
void describe_ground(int x, int y, map* cmap);
item** items_at(int x, int y, map* cmap);
int item_count_at(int x, int y, map* cmap);
void water_tile(int x, int y, map* cmap);
void get_random_empty_tile(int* x, int* y, map* cmap);
void get_tile_growth_info(int x, int y, float* w, float* n, float* m, map* cmap);
void update_tile_growth_info(int x, int y, float w, float n, float m, map* cmap);
bool is_down_stairs(int x, int y, map* cmap);
bool is_up_stairs(int x, int y, map* cmap);
void examine(int x, int y, map* cmap);

int lua_fertilize(lua_State* state);

#endif
