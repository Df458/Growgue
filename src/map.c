#include <stdlib.h>

#include "color.h"
#include "log.h"
#include "map.h"
#include "tile.h"

#define VIEW_W 78
#define VIEW_H 22
#define HVIEW_W VIEW_W / 2
#define HVIEW_H VIEW_H / 2

static WINDOW* map_win = 0;
static tile* tiles = 0;
static int width = 0;
static int height = 0;

void init_map(WINDOW* map)
{
    add_message(COLOR_WARNING, "Warning: init_map() stub!");
    map_win = map;

    width = VIEW_W;
    height = VIEW_H;
    tiles = calloc(width * height, sizeof(tile));
    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            tiles[i * width + j].display = rand() % 45 + 45;
            tiles[i * width + j].color = rand() % 5;
        }
    }
}

void update_map(int delta)
{
    add_message(COLOR_WARNING, "Warning: update_map() stub!");
    draw_map(0, 0);
}

void draw_map(int x, int y)
{
    int xmin = x - HVIEW_W < 0 ? 0 : x - HVIEW_W;
    int ymin = y - HVIEW_H < 0 ? 0 : y - HVIEW_H;
    int xmax = xmin + VIEW_W > width ? width : xmin + VIEW_W;
    int ymax = ymin + VIEW_H > height ? height : ymin + VIEW_H;
    wclear(map_win);
    set_color(map_win, COLOR_DEFAULT);
    wborder(map_win, 179, 186, 196, 205, 218, 210, 198, 202);
    mvwaddch(map_win, 7, 79, 204);
    mvwaddch(map_win, 15, 79, 204);
    mvwaddch(map_win, 23, 44, 209);
    mvwaddch(map_win, 23, 45, 209);

    for(int i = ymin; i < ymax; ++i) {
        wmove(map_win, i - ymin + 1, 1);
        for(int j = xmin; j < xmax; ++j) {
            set_color(map_win, tiles[i * width + j].color);
            waddch(map_win, tiles[i * width + j].display);
        }
    }

    wrefresh(map_win);
}
