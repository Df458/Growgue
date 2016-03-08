#ifndef MAP_H
#define MAP_H
#include <curses.h>
#include "tile.h"

typedef struct map
{
    tile* tiles;
    int width;
    int height;
}
map;

void init_map(WINDOW* map);
map* create_map();
void update_map(int delta, map* to_update);
void draw_map(int x, int y, map* to_draw);
bool can_move(int x, int y, map* to_move);

#endif
