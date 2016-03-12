#include <stdlib.h>

#include "color.h"
#include "log.h"
#include "luafunc.h"
#include "plant.h"
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
    new_map->actor_count = 0;
    new_map->actors = 0;

    for(int i = 0; i < new_map->height; ++i) {
        for(int j = 0; j < new_map->width; ++j) {
            new_map->tiles[i * new_map->width + j].display = rand() % 3 ? 219 : 178;
            new_map->tiles[i * new_map->width + j].color = 0;
            new_map->tiles[i * new_map->width + j].solid = true;
            new_map->tiles[i * new_map->width + j].actor_ref = 0;
            new_map->tiles[i * new_map->width + j].dist = 0;
            new_map->tiles[i * new_map->width + j].checked = false;
            new_map->tiles[i * new_map->width + j].can_till = false;
            new_map->tiles[i * new_map->width + j].tilled = false;
            new_map->tiles[i * new_map->width + j].item_count = 0;
            new_map->tiles[i * new_map->width + j].item_list = 0;
            new_map->tiles[i * new_map->width + j].water = 0;
            new_map->tiles[i * new_map->width + j].nutrients = 0;
            new_map->tiles[i * new_map->width + j].minerals = 0;
            new_map->tiles[i * new_map->width + j].plant_ref = 0;
        }
    }
    switch(gen_type) {
        case GEN_WALK: {
            int tilecount = width * height * 0.6 + (rand() % width - (width * 0.5));
            int i = 0;
            int cx[3];
            int csx = rand() % (width - 2) + 1;
            int cy[3];
            int csy = rand() % (height - 2) + 1;
            for(int j = 0; j < 3; ++j) {
                cx[j] = csx;
                cy[j] = csy;
            }
            while(tilecount > 0 && i < tilecount * 10) {
                for(int j = 0; j < 3; ++j) {
                    int index = cy[j] * width + cx[j];
                    if(new_map->tiles[index].solid == true)
                        tilecount--;
                    if(new_map->tiles[index].solid == false && !rand() % 3) {
                        cx[j] = rand() % (width - 2) + 1;
                        cy[j] = rand() % (width - 2) + 1;
                        index = cy[j] * width + cx[j];
                    }
                    new_map->tiles[index].display = '.';
                    new_map->tiles[index].color = 0;
                    new_map->tiles[index].solid = false;
                    new_map->tiles[index].actor_ref = 0;
                    i++;

                    int cnx = cx[j];
                    int cny = cy[j];
                    if(rand() % 2)
                        cnx = cx[j] + (rand() % 2 ? 1 : -1);
                    else
                        cny = cy[j] + (rand() % 2 ? 1 : -1);
                    cx[j] = clamp(cnx, 1, width - 2);
                    cy[j] = clamp(cny, 1, height - 2);
                }
            }
            } break;
    }

    int patch_count = rand() % 8 + 2;
    for(int i = 0; i < patch_count; ++i) {
        int cx, cy;
        get_random_empty_tile(&cx, &cy, new_map);

        int size = rand() % 5 + 1;
        for(int j = clamp(cy - size, 1, height - 2); j < clamp(cy + size, 1, height - 2); ++j) {
            for(int k = clamp(cx - size, 1, width - 2); k < clamp(cx + size, 1, width - 2); ++k) {
                new_map->tiles[j * width + k].display = rand() % 2 ? '.' : ',';
                new_map->tiles[j * width + k].color = COLOR_SOIL;
                new_map->tiles[j * width + k].solid = false;
                new_map->tiles[j * width + k].can_till = true;
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
    for(int i = 0; i < to_update->height; ++i) {
        for(int j = 0; j < to_update->width; ++j) {
            if(to_update->tiles[i * to_update->width + j].plant_ref) {
                update_plant(to_update->tiles[i * to_update->width + j].plant_ref, delta);
                if(to_update->tiles[i * to_update->width + j].plant_ref->dead)
                    kill_plant(to_update->tiles[i * to_update->width + j].plant_ref);
            }
        }
    }
    draw_map(x, y, to_update);
}

void destroy_map(map* to_destroy)
{
    for(int i = 0; i < to_destroy->actor_count; ++i) {
        if(to_destroy->actors[i]) {
            kill_actor(to_destroy->actors[i]);
        }
    }
    free(to_destroy->actors);
    free(to_destroy->tiles);
    free(to_destroy);
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
            } else if(to_draw->tiles[i * to_draw->width + j].plant_ref) {
                set_color(map_win, to_draw->tiles[i * to_draw->width + j].plant_ref->color);
                waddch(map_win, to_draw->tiles[i * to_draw->width + j].plant_ref->display);
            } else {
                int k;
                for(k = 0; k < to_draw->tiles[i * to_draw->width + j].item_count; ++k)
                    if(to_draw->tiles[i * to_draw->width + j].item_list[k] != 0) {
                        set_color(map_win, to_draw->tiles[i * to_draw->width + j].item_list[k]->color);
                        waddch(map_win, to_draw->tiles[i * to_draw->width + j].item_list[k]->display);
                        break;
                    }
                if(k == to_draw->tiles[i * to_draw->width + j].item_count) {
                    set_color(map_win, to_draw->tiles[i * to_draw->width + j].color);
                    waddch(map_win, to_draw->tiles[i * to_draw->width + j].display);
                }
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

bool spawn_item(int x, int y, const char* file, map* to_spawn)
{
    if(can_move(x, y, to_spawn)) {
        item* it = create_item(file);
        if(!it) {
            fprintf(stderr, "Couldn't spawn item");
            return false;
        }
        place_item(x, y, it, to_spawn);
    }
    return false;
}

bool spawn_plant(int x, int y, const char* file, map* to_spawn)
{
    plant* it = create_plant(file, x, y, to_spawn);
    if(!it) {
        fprintf(stderr, "Couldn't spawn plant");
        return false;
    }
    to_spawn->tiles[y * to_spawn->width + x].plant_ref = it;
    return true;
}

bool can_plant(int x, int y, map* cmap, bool explain)
{
    int id = y * cmap->width + x;
    if(!cmap->tiles[id].can_till) {
        if(explain)
            add_message(COLOR_WARNING, "There's no soil for planting here!");
        return false;
    }
    if(!cmap->tiles[id].tilled) {
        if(explain)
            add_message(COLOR_WARNING, "The soil here needs to be tilled.");
        return false;
    }
    if(cmap->tiles[id].plant_ref) {
        if(explain)
            add_message(COLOR_WARNING, "There's already a plant here!");
        return false;
    }
    return true;
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
    }
    return false;
}

actor* get_actor_at(int x, int y, map* cmap)
{
    return cmap->tiles[y * cmap->width + x].actor_ref;
}

void till(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;
    if(!cmap->tiles[id].can_till) {
        add_message(COLOR_WARNING, "There is no soil to till here!");
        return;
    }
    if(cmap->tiles[id].tilled) {
        add_message(COLOR_WARNING, "The soil here is already tilled!");
        return;
    }
    cmap->tiles[id].tilled = true;
    cmap->tiles[id].display = 247;
    add_message(COLOR_DEFAULT, "You till the soil.");
}

int get_cost(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;
    return cmap->tiles[id].cost;
}

void place_item(int x, int y, item* it, map* cmap)
{
    it->x = x;
    it->y = y;
    it->on_ground = true;

    int id = y * cmap->width + x;

    tile* t = &cmap->tiles[id];
    int i;
    for(i = 0; i < t->item_count; ++i) {
        if(t->item_list[i] == 0)
            break;
    }
    if(i == t->item_count) {
        t->item_count += 5;
        t->item_list = realloc(t->item_list, t->item_count * sizeof(item*));
        for(int j = i; j < i + 5; ++j) {
            t->item_list[j] = 0;
        }
    }
    t->item_list[i] = it;
}

void take_item(int x, int y, item* it, map* cmap)
{
    int id = y * cmap->width + x;
    for(int i = 0; i < cmap->tiles[id].item_count; ++i) {
        if(cmap->tiles[id].item_list[i] == it)
            cmap->tiles[id].item_list[i] = 0;
    }
}

item* harvest_plant(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;
    if(!cmap->tiles[id].plant_ref) {
        add_message(COLOR_WARNING, "There's no plant to harvest here!");
        return 0;
    }
    if(!cmap->tiles[id].plant_ref->can_harvest) {
        add_message(COLOR_WARNING, "This plant can't be harvested!");
        return 0;
    }
    if(cmap->tiles[id].plant_ref->growth_time > 0) {
        add_message(COLOR_WARNING, "This plant isn't ready for harvest yet!");
        return 0;
    }
    item* it = create_item(cmap->tiles[id].plant_ref->item_id);
    if(it) {
        it->count = cmap->tiles[id].plant_ref->item_count;
    }
    kill_plant(cmap->tiles[id].plant_ref);
    return it;
}

void describe_ground(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;

    tile* t = &cmap->tiles[id];

    for(int i = 0; i < t->item_count; ++i) {
        if(t->item_list[i] != 0) {
            printf_message(COLOR_DEFAULT, "You see %d %s on the ground here", t->item_list[i]->count, t->item_list[i]->name);
            callback("step", t->item_list[i]->script_state);
        }
    }
    if(t->plant_ref) {
        printf_message(COLOR_DEFAULT, "There is a %s planted here", t->plant_ref->name);
        if(t->plant_ref->can_harvest && t->plant_ref->growth_time <= 0)
            printf_message(COLOR_DEFAULT, "It's ready for harvest");
        callback("step", t->plant_ref->script_state);
    }
}

item** items_at(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;
    return cmap->tiles[id].item_list;
}

int item_count_at(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;
    return cmap->tiles[id].item_count;
}

void water_tile(int x, int y, map* cmap)
{
    int id = y * cmap->width + x;

    cmap->tiles[id].water = 100;
    if(!cmap->tiles[id].can_till)
        add_message(COLOR_DEFAULT, "You pour water on the ground");
    else if(cmap->tiles[id].plant_ref)
        add_message(COLOR_DEFAULT, "You water the plant");
    else
        add_message(COLOR_DEFAULT, "You water the soil");
}

void get_random_empty_tile(int* x, int* y, map* cmap)
{
    int cx, cy;

    do {
        cx = rand() % (cmap->width - 2) + 1;
        cy = rand() % (cmap->height - 2) + 1;
    } while(cmap->tiles[cy * cmap->width + cx].solid == true);
    *x = cx;
    *y = cy;
}

void get_tile_growth_info(int x, int y, float* w, float* n, float* m, map* cmap)
{
    int index = y * cmap->width + x;
    *w = cmap->tiles[index].water;
    *n = cmap->tiles[index].nutrients;
    *m = cmap->tiles[index].minerals;
}

void update_tile_growth_info(int x, int y, float w, float n, float m, map* cmap)
{
    int index = y * cmap->width + x;
    cmap->tiles[index].water -= w;
    cmap->tiles[index].nutrients -= n;
    cmap->tiles[index].minerals -= m;
}
