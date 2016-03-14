#define true TRUE
#define false FALSE
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "controls.h"
#include "input.h"
#include "log.h"
#include "luafunc.h"
#include "player.h"
#include "reqs.h"

static WINDOW* map_win = 0;
static WINDOW* stats_win = 0;
static WINDOW* area_win = 0;
static WINDOW* examine_win = 0;
static WINDOW* hp_win = 0;

#define BAR_LENGTH 43

static int hp_max = 0;
static int ep_max = 0;
static int str = 0;
static int def = 0;

static int xp = 0;
static int next = 15;
static int level = 1;

static item* equipment[3] = { 0 };

static int hp_current;
static int ep_current;

static int x = 0;
static int y = 0;
static int pres_x = 0;
static int pres_y = 0;
static int z = 0;

static map* current_map = 0;
static item** inventory = 0;
static int item_count = 0;

const char* slot_names[SLOT_COUNT] = { "Head", "Body", "Hands" };

void init_stats()
{
    hp_max = 50;
    ep_max = 100;

    hp_current = hp_max;
    ep_current = ep_max;

    str = 5;
    def = 0;

    xp = 0;
    next = 25;
    level = 1;
}

void init_player(WINDOW* mapw, WINDOW* stats, WINDOW* hp, WINDOW* area, WINDOW* examine, map* start_map)
{
    map_win = mapw;
    stats_win = stats;
    hp_win = hp;
    area_win = area;
    examine_win = examine;

    init_stats();

    current_map = start_map;
}

void draw_player(int py, int px)
{
    wborder(hp_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    if(hp_current > 0) {
        float ratio = (float)hp_current / (float)hp_max;
        int len = BAR_LENGTH * ratio;
        if(len == 0)
            len = 1;
        else if(len > BAR_LENGTH)
            len = BAR_LENGTH;
        if(ratio > 0.6f)
            set_color(hp_win, COLOR_HP_GOOD);
        else if(ratio > 0.3f)
            set_color(hp_win, COLOR_HP_LOW);
        else
            set_color(hp_win, COLOR_HP_CRIT);
        char* life_str = malloc(sizeof(char) * BAR_LENGTH + 1);
        memset(life_str, '=', sizeof(char) * len);
        memset(life_str + len, ' ', sizeof(char) * (BAR_LENGTH - len));
        mvwaddstr(hp_win, 0, 1, life_str);
        free(life_str);
    }
    if(ep_current > 0) {
        float ratio = (float)ep_current / (float)ep_max;
        int len = BAR_LENGTH * ratio;
        if(len == 0)
            len = 1;
        else if(len > BAR_LENGTH)
            len = BAR_LENGTH;
        if(ratio > 0.6f)
            set_color(hp_win, COLOR_EP_GOOD);
        else if(ratio > 0.3f)
            set_color(hp_win, COLOR_EP_LOW);
        else
            set_color(hp_win, COLOR_EP_CRIT);
        char* life_str = malloc(sizeof(char) * BAR_LENGTH + 1);
        memset(life_str, '=', sizeof(char) * len);
        memset(life_str + len, ' ', sizeof(char) * (BAR_LENGTH - len));
        mvwaddstr(hp_win, 0, BAR_LENGTH + 3, life_str);
        free(life_str);
    }

    set_color(hp_win, COLOR_DEFAULT);
    mvwaddch(hp_win, 0, 0, 179);
    mvwaddch(hp_win, 0, 44, 179);
    mvwaddch(hp_win, 0, 45, 179);
    mvwaddch(hp_win, 0, 89, 179);

    wborder(stats_win, ' ', 179, ' ', 205, ' ', 179, 205, 181);
    mvwprintw(stats_win, 0, 0, "HP: %-5d", hp_max);
    mvwprintw(stats_win, 1, 0, "EP: %-5d", ep_max);
    mvwprintw(stats_win, 3, 0, "STR: %-4d", str);
    mvwprintw(stats_win, 4, 0, "DEF: %-4d", def);

    move(px, py);
    mvwaddch(map_win, py, px, '@');
    pres_x = px;
    pres_y = py;
    mvwprintw(map_win, 23, 2, "LEVEL: %02d", level);
    mvwprintw(map_win, 23, 13, "FLOOR: %02d", z + 1);

    wclear(examine_win);
    wborder(examine_win, ' ', 179, ' ', 205, ' ', 179, 205, 181);
    if(equipment[0]) {
        mvwprintw(examine_win, 6, 0, "Head:");
        mvwprintw(examine_win, 7, 0, "%.9s", equipment[0]->name);
        mvwprintw(examine_win, 8, 0, "+%d +%d", equipment[0]->str, equipment[0]->def);
    } else {
        mvwprintw(examine_win, 6, 0, "Head:");
        mvwprintw(examine_win, 7, 0, "None");
    }
    if(equipment[1]) {
        mvwprintw(examine_win, 3, 0, "Body:");
        mvwprintw(examine_win, 4, 0, "%.9s", equipment[1]->name);
        mvwprintw(examine_win, 5, 0, "+%d +%d", equipment[1]->str, equipment[1]->def);
    } else {
        mvwprintw(examine_win, 3, 0, "Body:");
        mvwprintw(examine_win, 4, 0, "None");
    }
    if(equipment[2]) {
        mvwprintw(examine_win, 0, 0, "Weapon:");
        mvwprintw(examine_win, 1, 0, "%.9s", equipment[2]->name);
        mvwprintw(examine_win, 2, 0, "+%d +%d", equipment[2]->str, equipment[2]->def);
    } else {
        mvwprintw(examine_win, 0, 0, "Weapon:");
        mvwprintw(examine_win, 1, 0, "None");
    }
    wborder(area_win, ' ', 179, 196, 205, 196, 191, 205, 181);

    wrefresh(map_win);
    wrefresh(stats_win);
    wrefresh(hp_win);
    wrefresh(examine_win);
    wrefresh(area_win);
}

void update_player()
{
    switch(get_last_input()) {
        case INPUT_DIRECTIONAL:
            player_move(get_last_direction());
            break;
        case INPUT_ACTION:
            player_act();
            break;
    }
    if(hp_current < hp_max)
        hp_current++;
    if(hp_current > hp_max)
        hp_current = hp_max;
    if(ep_current > ep_max)
        ep_current = ep_max;
}

void cleanup_player()
{
   delwin(map_win);
   delwin(stats_win);
   delwin(hp_win);
   delwin(area_win);
   delwin(examine_win);
}

void player_get_position(int* px, int* py)
{
    *px = x;
    *py = y;
}

void player_set_position(int px, int py)
{
    x = px;
    y = py;
}

void player_move(int direction)
{
    int dx = 0;
    int dy = 0;
    switch(direction) {
        case DIRECTION_NORTH:
            dy = -1;
            break;
        case DIRECTION_SOUTH:
            dy = 1;
            break;
        case DIRECTION_EAST:
            dx = 1;
            break;
        case DIRECTION_WEST:
            dx = -1;
            break;
        case DIRECTION_NORTHEAST:
            dx = 1;
            dy = -1;
            break;
        case DIRECTION_NORTHWEST:
            dx = -1;
            dy = -1;
            break;
        case DIRECTION_SOUTHEAST:
            dx = 1;
            dy = 1;
            break;
        case DIRECTION_SOUTHWEST:
            dx = -1;
            dy = 1;
            break;
        case DIRECTION_UP:
            if(z > 0 && is_up_stairs(x, y, current_map))
                z--;
            return;
        case DIRECTION_DOWN:
            if(z < LEVEL_COUNT - 1 && is_down_stairs(x, y, current_map))
                z++;
            return;
    }
    int res = can_move(x + dx, y + dy, current_map);
    if(res == 1) {
        x += dx;
        y += dy;
        int cost = 1 + get_cost(x, y, current_map);
        if(dx != 0 && dy != 0)
            cost *= 1.5;
        ep_current -= cost;
        if(ep_current <= 0)
            add_message(COLOR_EP_CRIT, "Out of energy, you fall to the ground.");
        else
            describe_ground(x, y, current_map);
    } else if(res == 2) {
        actor* act = get_actor_at(x + dx, y + dy, current_map);
        int astr = str;
        for(int i = 0; i < SLOT_COUNT; ++i)
            if(equipment[i])
                astr += equipment[i]->str;
        int dmg = damage_actor(act, astr);
        char* damage_text = 0;
        if(dmg > 0) {
            int len = snprintf(0, 0, "You hit the %s for %d damage", act->name, dmg);
            damage_text = calloc(len + 1, sizeof(char));
            snprintf(damage_text, len + 1, "You hit the %s for %d damage", act->name, dmg);
        } else {
            int len = snprintf(0, 0, "You miss the %s", act->name);
            damage_text = calloc(len + 1, sizeof(char));
            snprintf(damage_text, len + 1, "You miss the %s", act->name);
        }
        if(act->hp <= 0) {
            printf_message(COLOR_DEFAULT, "You kill the %s!", act->name);
            add_xp(act->xp);
        } else
            add_message(COLOR_DEFAULT, damage_text);
        free(damage_text);
    }
}

void player_act()
{
    switch(get_last_action()) {
        case ACTION_TILL:
            till(x, y, current_map);
            break;
        case ACTION_PICKUP: {
            item* it = get_item(items_at(x, y, current_map), item_count_at(x, y, current_map), PURPOSE_PICKUP, true);
            if(!it)
                break;
            printf_message(COLOR_DEFAULT, "Picked up %d %s", it->count, it->name);
            callback("picked_up", it->script_state);
            take_item(x, y, it, current_map);
            add_item(it);
        } break;
        case ACTION_DROP: {
            item* it = get_item(inventory, item_count, PURPOSE_DROP, false);
            if(!it)
                break;
            if(it->can_equip && equipment[it->slot] == it) {
                equipment[it->slot] = 0;
                printf_message(COLOR_DEFAULT, "You unequip the %s, and drop it on the ground.", it->name);
                callback("removed", it->script_state);
            }
            item* clone = clone_item(it);
            callback("dropped", clone->script_state);
            place_item(x, y, clone, current_map);
            remove_item(it, -1);
        } break;
        case ACTION_APPLY: {
            item* it = get_item(inventory, item_count, PURPOSE_APPLY, false);
            if(!it)
                break;
            callback("apply", it->script_state);
            remove_item(it, 1);
        } break;
        case ACTION_EQUIP: {
            item* it = get_item(inventory, item_count, PURPOSE_EQUIP, false);
            if(!it || it->slot == SLOT_INVALID)
                break;
            callback("equip", it->script_state);
            printf_message(COLOR_DEFAULT, "You equip the %s.", it->name);
            equipment[it->slot] = it;
        break; }
        case ACTION_REMOVE: {
            item* it = get_item(equipment, 3, PURPOSE_REMOVE, false);
            if(!it)
                break;
            callback("remove", it->script_state);
            equipment[it->slot] = 0;
            printf_message(COLOR_DEFAULT, "You unequip the %s.", it->name);
        break; }
        case ACTION_PLANT: {
            if(!can_plant(x, y, current_map, true))
                break;
            item* it = get_item(inventory, item_count, PURPOSE_PLANT, false);
            if(!it)
                break;
            if(spawn_plant(x, y, it->plant_id, current_map)) {
                printf_message(COLOR_DEFAULT, "You plant the %s in the tilled soil.", it->name);
                remove_item(it, 1);
            }
        break; }
        case ACTION_HARVEST: {
            add_item(harvest_plant(x, y, current_map));
        break; }
        case ACTION_HELP:
            show_controls();
            break;
        case ACTION_INVENTORY:
            get_item(inventory, item_count, PURPOSE_NONE, false);
            break;
        case ACTION_WATER:
            water_tile(x, y, current_map);
            break;
        case ACTION_EXAMINE: {
            int mx, my;
            get_last_mouse_position(&mx, &my);
            int xdiff = mx - pres_x;
            int ydiff = my - pres_y;
            if(mx < 1 || my < 1 || mx > 78 || my > 78)
                break;
            examine(x + xdiff, y +ydiff, current_map);
        } break;
    }
    if(ep_current <= 0)
        add_message(COLOR_EP_CRIT, "Out of energy, you fall to the ground.");
}

int damage_player(int damage)
{
    int adef = def;
    for(int i = 0; i < SLOT_COUNT; ++i)
        if(equipment[i]) {
            adef += equipment[i]->def;
            callback("attacked", equipment[i]->script_state);
        }
    hp_current -= damage - adef;
    return damage - adef;
}

bool is_dead()
{
    return hp_current <= 0 || ep_current <= 0;
}

void add_item(item* it)
{
    if(it == 0)
        return;
    int index = -1;
    for(int i = 0; i < item_count; ++i) {
        if(index == -1 && inventory[i] == 0) {
            index = i;
            inventory[index] = it;
            break;
        } else if(inventory[i] && !strcmp(it->orig_path, inventory[i]->orig_path)) {
            inventory[i]->count += it->count;
            destroy_item(it);
            return;
        }
    }

    if(index == -1) {
        inventory = realloc(inventory, (item_count + 5) * sizeof(item*));
        for(int i = item_count + 1; i < item_count + 5; ++i)
            inventory[i] = 0;
        inventory[item_count] = it;
        item_count += 5;
        it->on_ground = false;
    }
}

void remove_item(item* item, int quantity)
{
    if(item == 0)
        return;
    for(int i = 0; i < item_count; ++i) {
        if(inventory[i] == item) {
            if(item->count <= quantity || quantity == -1) {
                destroy_item(item);
                inventory[i] = 0;
            } else
                item->count -= quantity;
            break;
        }
    }
}

int get_current_floor()
{
    return z;
}

void set_current_map(map* map, bool down, bool up)
{
    current_map = map;
    if(down) {
        x = current_map->us_x;
        y = current_map->us_y;
    } else if(up) {
        x = current_map->ds_x;
        y = current_map->ds_y;
    }
}

void add_xp(int new_xp)
{
    xp += new_xp;

    while(xp >= next) {
        level += 1;
        xp -= next;
        next = 15 + (20 * level) + (5 * pow((level + 1) / 2, 2));
        hp_max += 25;
        ep_max += 50;
        hp_current += 25;
        ep_current += 50;
        str += 2;
        def += 1;
    }
}

WINDOW* get_map_window()
{
    return map_win;
}

int lua_player_index(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    if(!strcmp(index, "hp")) lua_pushinteger(state, hp_current);
    else if(!strcmp(index, "hp_max")) lua_pushinteger(state, hp_max);
    else if(!strcmp(index, "ep")) lua_pushinteger(state, ep_current);
    else if(!strcmp(index, "ep_max")) lua_pushinteger(state, ep_max);
    else if(!strcmp(index, "str")) lua_pushinteger(state, str);
    else if(!strcmp(index, "def")) lua_pushinteger(state, def);
    else if(!strcmp(index, "x")) lua_pushinteger(state, x);
    else if(!strcmp(index, "y")) lua_pushinteger(state, y);
    else return 0;
    return 1;
}

void update_hp(int hp)
{
    if(hp > hp_current) {
        printf_message(COLOR_HP_GOOD, "You gain %d hp.", hp - hp_current);
    } else if(hp < hp_current) {
        printf_message(COLOR_HP_CRIT, "You lose %d hp.", hp_current - hp, hp_current, hp);
    }
    hp_current = hp;
}

int lua_player_newindex(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    if(!strcmp(index, "hp") && (lua_isinteger(state, 3) || lua_isnumber(state, 3))) { update_hp(lua_tointeger(state, 3)); }
    else if(!strcmp(index, "hp_max")) hp_max = lua_tointeger(state, 3);
    else if(!strcmp(index, "ep")) ep_current = lua_tointeger(state, 3);
    else if(!strcmp(index, "ep_max")) ep_max = lua_tointeger(state, 3);
    else if(!strcmp(index, "str")) str = lua_tointeger(state, 3);
    else if(!strcmp(index, "def")) def = lua_tointeger(state, 3);
    else if(!strcmp(index, "x")) x = lua_tointeger(state, 3);
    else if(!strcmp(index, "y")) y = lua_tointeger(state, 3);
    return 0;
}

void insert_player_into_lua(lua_State* state)
{
    lua_newtable(state);
    luaL_setfuncs(state, player_funcs, 0);
    lua_newtable(state);
    luaL_setfuncs(state, player_meta, 0);
    lua_setmetatable(state, 1);
    lua_setglobal(state, "player");
}

int lua_fertilize(lua_State* state)
{
    current_map->tiles[y * current_map->width + x].nutrients = 100;
    return 0;
}
