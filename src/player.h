#ifndef PLAYER_H
#define PLAYER_H
#include <curses.h>

void init_player(WINDOW* map, WINDOW* stats, WINDOW* hp);
void update_player();
void draw_player();
void cleanup_player();

#endif
