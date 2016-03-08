#ifndef PLAYER_H
#define PLAYER_H
#include <curses.h>
#include "map.h"

void init_player(WINDOW* mapw, WINDOW* stats, WINDOW* hp, map* start_map);
void update_player();
void draw_player(int x, int y);
void cleanup_player();
void player_get_position(int* px, int* py);
void player_move(int direction);

#endif
