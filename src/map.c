#include <stdlib.h>

#include "color.h"
#include "log.h"
#include "player.h"
#include "map.h"

#define VIEW_W 78
#define VIEW_H 22
#define HVIEW_W VIEW_W / 2
#define HVIEW_H VIEW_H / 2

static WINDOW* map_win = 0;
/* static tile* tiles = 0; */
/* static int width = 0; */
/* static int height = 0; */

void init_map(WINDOW* map)
{
    map_win = map;
}

map* create_map(int width, int height)
{
    map* new_map = malloc(sizeof(map));

    new_map->width = width;
    new_map->height = height;
    new_map->tiles = calloc(new_map->width * new_map->height, sizeof(tile));
    for(int i = 0; i < new_map->height; ++i) {
        for(int j = 0; j < new_map->width; ++j) {
            new_map->tiles[i * new_map->width + j].display = rand() % 45 + 45;
            new_map->tiles[i * new_map->width + j].color = rand() % 5;
            new_map->tiles[i * new_map->width + j].solid = false;
        }
    }

    return new_map;
}

void update_map(int delta, map* to_update)
{
    add_message(COLOR_WARNING, "Warning: update_map() stub!");
    int x, y;
    player_get_position(&x, &y);
    draw_map(x, y, to_update);
}

void draw_map(int x, int y, map* to_draw)
{
    int xmin = x - HVIEW_W < 0 ? 0 : (x + HVIEW_W - 1 >= to_draw->width && to_draw->width >= VIEW_W ? to_draw->width - VIEW_W : x - HVIEW_W);
    int ymin = y - HVIEW_H < 0 ? 0 : (y + HVIEW_H - 1 >= to_draw->height && to_draw->height >= VIEW_H ? to_draw->height - VIEW_H : y - HVIEW_H);
    int xmax = xmin + VIEW_W >= to_draw->width ? to_draw->width : xmin + VIEW_W;
    int ymax = ymin + VIEW_H >= to_draw->height ? to_draw->height : ymin + VIEW_H;
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
            set_color(map_win, to_draw->tiles[i * to_draw->width + j].color);
            waddch(map_win, to_draw->tiles[i * to_draw->width + j].display);
        }
    }

    set_color(map_win, COLOR_DEFAULT);
    draw_player(y - ymin + 1, x - xmin + 1);

    wrefresh(map_win);
}

bool can_move(int x, int y, map* to_move)
{
    // TODO: Take entities into account
    if(x < 0 || y < 0 || x >= to_move->width || y >= to_move->height)
        return false;
    return !to_move->tiles[y * to_move->width + x].solid;
}
