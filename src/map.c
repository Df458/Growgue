#include <stdlib.h>

#include "color.h"
#include "log.h"
#include "player.h"
#include "macro.h"
#include "map.h"
#include "queue.h"

#define VIEW_W 78
#define VIEW_H 22
#define HVIEW_W VIEW_W / 2
#define HVIEW_H VIEW_H / 2

static WINDOW* map_win = 0;

void init_map(WINDOW* map)
{
    map_win = map;
}

map* create_map(int width, int height, int gen_type)
{
    map* new_map = malloc(sizeof(map));

    new_map->width = width;
    new_map->height = height;
    new_map->tiles = calloc(new_map->width * new_map->height, sizeof(tile));

    switch(gen_type) {
        case GEN_WALK:
            // TODO: Drunkard's walk
            break;
    }
    for(int i = 0; i < new_map->height; ++i) {
        for(int j = 0; j < new_map->width; ++j) {
            new_map->tiles[i * new_map->width + j].display = '.';
            new_map->tiles[i * new_map->width + j].color = 0;
            new_map->tiles[i * new_map->width + j].solid = false;
            new_map->tiles[i * new_map->width + j].actor_ref = 0;
            new_map->tiles[i * new_map->width + j].dist = 0;
            new_map->tiles[i * new_map->width + j].checked = false;
            if(rand() % 3 == 0) {
                new_map->tiles[i * new_map->width + j].display = '#';
                new_map->tiles[i * new_map->width + j].solid = true;
            }
        }
    }

    return new_map;
}

void update_map(int delta, map* to_update)
{
    int x, y;
    player_get_position(&x, &y);
    set_weights(x, y, to_update);
    for(int i = 0; i < to_update->actor_count; ++i) {
        if(!to_update->actors[i])
            continue;
        update_actor(to_update->actors[i], to_update);
        if(to_update->actors[i]->to_kill) {
            to_update->tiles[to_update->actors[i]->y * to_update->width + to_update->actors[i]->x].actor_ref = 0;
            kill_actor(to_update->actors[i]);
            to_update->actors[i] = 0;
        }
    }
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
            if(to_draw->tiles[i * to_draw->width + j].actor_ref) {
                set_color(map_win, to_draw->tiles[i * to_draw->width + j].actor_ref->color);
                waddch(map_win, to_draw->tiles[i * to_draw->width + j].actor_ref->display);
            } else {
                /* if(to_draw->tiles[i * to_draw->width + j].dist <= 9 && to_draw->tiles[i * to_draw->width + j].dist >= 0) { */
                /*     set_color(map_win, to_draw->tiles[i * to_draw->width + j].color); */
                /*     waddch(map_win, '0' + to_draw->tiles[i * to_draw->width + j].dist); */
                /* } else { */
                set_color(map_win, to_draw->tiles[i * to_draw->width + j].color);
                waddch(map_win, to_draw->tiles[i * to_draw->width + j].display);
                /* } */
            }
        }
    }

    set_color(map_win, COLOR_DEFAULT);
    draw_player(y - ymin + 1, x - xmin + 1);

    wrefresh(map_win);
}

int can_move(int x, int y, map* to_move)
{
    int px, py;
    player_get_position(&px, &py);
    if(px == x && py == y)
        return 0;
    if(x < 0 || y < 0 || x >= to_move->width || y >= to_move->height)
        return 0;
    if(to_move->tiles[y * to_move->width + x].solid)
        return 0;
    if(to_move->tiles[y * to_move->width + x].actor_ref)
        return 2;
    return 1;
}

bool spawn_actor(int x, int y, const char* file, map* to_spawn)
{
    if(can_move(x, y, to_spawn)) {
        actor* act = create_actor(file);
        to_spawn->tiles[y * to_spawn->width + x].actor_ref = act;
        act->x = x;
        act->y = y;
        init_actor(act);
        bool found = false;
        for(int i = 0; i < to_spawn->actor_count; ++i) {
            if(to_spawn->actors[i] == 0) {
                to_spawn->actors[i] = act;
                found = true;
            }
        }
        if(!found) {
            // TODO: Resize
            to_spawn->actors = realloc(to_spawn->actors, (to_spawn->actor_count + 10) * sizeof(actor*));
            to_spawn->actors[to_spawn->actor_count] = act;
            for(int i = to_spawn->actor_count + 1; i < to_spawn->actor_count + 10; ++i)
                to_spawn->actors[i] = 0;
            to_spawn->actor_count += 10;
        }
        return true;
    }
    return false;
}

struct coord_pair { int x; int y; };

void add_neightbors(queue* s, int x, int y, map* to_update)
{
    for(int i = clamp(y - 1, 0, to_update->height - 1); i <= clamp(y + 1, 0, to_update->height - 1); ++i)
        for(int j = clamp(x - 1, 0, to_update->width - 1); j <= clamp(x + 1, 0, to_update->width - 1); ++j)
            if(!to_update->tiles[i * to_update->width + j].added && !to_update->tiles[i * to_update->width + j].checked && !to_update->tiles[i * to_update->width + j].solid) {
                to_update->tiles[i * to_update->width + j].added = true;
                struct coord_pair* p = malloc(sizeof(struct coord_pair));
                p->x = j;
                p->y = i;
                queue_enqueue(s, p);
            }
}

void set_weights(int x, int y, map* to_update)
{
    for(int i = 0; i < to_update->height * to_update->width; ++i) {
        to_update->tiles[i].dist = -1;
        to_update->tiles[i].checked = false;
        to_update->tiles[i].added = false;
    }
    to_update->tiles[y * to_update->width + x].checked = true;
    to_update->tiles[y * to_update->width + x].added = true;
    to_update->tiles[y * to_update->width + x].dist = 0;
    // TODO: Use a queue
    queue* location_queue = queue_make();
    add_neightbors(location_queue, x, y, to_update);
    while(!queue_empty(location_queue)) {
        struct coord_pair* p = (struct coord_pair*)queue_dequeue(location_queue, 0);
        for(int i = clamp(p->y - 1, 0, to_update->height - 1); i <= clamp(p->y + 1, 0, to_update->height - 1); ++i)
            for(int j = clamp(p->x - 1, 0, to_update->width - 1); j <= clamp(p->x + 1, 0, to_update->width - 1); ++j) {
                if((to_update->tiles[i * to_update->width + j].dist < to_update->tiles[p->y * to_update->width + p->x].dist || to_update->tiles[p->y * to_update->width + p->x].dist == -1) && to_update->tiles[i * to_update->width + j].checked == true) {
                    to_update->tiles[p->y * to_update->width + p->x].dist = to_update->tiles[i * to_update->width + j].dist + 1;
                    /* to_update->tiles[p->y * to_update->width + p->x].dist = 1; */
                }
            }
        to_update->tiles[p->y * to_update->width + p->x].checked = true;
        add_neightbors(location_queue, p->x, p->y, to_update);
        free(p);
    }
}

bool step_towards_player(actor* act, map* cmap)
{
    struct coord_pair pos;
    pos.x = act->x;
    pos.y = act->y;
    int cost = cmap->tiles[act->y * cmap->width + act->x].cost;
    for(int i = clamp(act->y - 1, 0, cmap->height - 1); i <= clamp(act->y + 1, 0, cmap->height - 1); ++i)
        for(int j = clamp(act->x - 1, 0, cmap->width - 1); j <= clamp(act->x + 1, 0, cmap->width - 1); ++j) {
            if(!cmap->tiles[i * cmap->width + j].solid && (cmap->tiles[i * cmap->width + j].dist < cost || cost == 0)) {
                pos.x = j;
                pos.y = i;
                cost = cmap->tiles[i * cmap->width + j].dist;
            }
        }
    if(can_move(pos.x, pos.y, cmap) == 1) {
        cmap->tiles[act->y * cmap->width + act->x].actor_ref = 0;
        act->x = pos.x;
        act->y = pos.y;
        cmap->tiles[act->y * cmap->width + act->x].actor_ref = act;
        return true;
    } else
        fprintf(stderr, ":(\n)");
    return false;
}

actor* get_actor_at(int x, int y, map* cmap)
{
    return cmap->tiles[y * cmap->width + x].actor_ref;
}
