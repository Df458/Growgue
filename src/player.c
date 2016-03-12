#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "input.h"
#include "log.h"
#include "luafunc.h"
#include "player.h"

static WINDOW* map_win = 0;
static WINDOW* stats_win = 0;
static WINDOW* hp_win = 0;

#define BAR_LENGTH 43

static int hp_max = 0;
static int ep_max = 0;
static int str = 0;
static int def = 0;

static item* equipment[3] = { 0 };

static int hp_current;
static int ep_current;

static int x = 0;
static int y = 0;

static map* current_map = 0;
static item** inventory = 0;
static int item_count = 0;

const char* slot_names[SLOT_COUNT] = { "Head", "Body", "Hands" };

void init_stats()
{
    hp_max = 100;
    ep_max = 100;

    hp_current = hp_max;
    ep_current = ep_max;
}

void init_player(WINDOW* mapw, WINDOW* stats, WINDOW* hp, map* start_map)
{
    map_win = mapw;
    stats_win = stats;
    hp_win = hp;

    init_stats();

    current_map = start_map;
}

void draw_player(int x, int y)
{
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

    mvwprintw(stats_win, 0, 0, "HP: %-5d", hp_max);
    mvwprintw(stats_win, 1, 0, "EP: %-5d", ep_max);
    mvwprintw(stats_win, 3, 0, "STR: %-4d", str);
    mvwprintw(stats_win, 4, 0, "DEF: %-4d", def);

    mvwaddch(map_win, x, y, '@');

    wrefresh(map_win);
    wrefresh(stats_win);
    wrefresh(hp_win);
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
}

void cleanup_player()
{
   delwin(map_win);
   delwin(stats_win);
   delwin(hp_win);
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
            take_item(x, y, it, current_map);
            printf_message(COLOR_DEFAULT, "Picked up %d %s", it->count, it->name);
            add_item(it);
            callback("picked_up", it->script_state);
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
            place_item(x, y, clone, current_map);
            callback("dropped", clone->script_state);
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
            item* it = get_item(inventory, 3, PURPOSE_EQUIP, false);
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
        case ACTION_WATER:
            water_tile(x, y, current_map);
            break;
    }
    if(ep_current <= 0)
        add_message(COLOR_EP_CRIT, "Out of energy, you fall to the ground.");
}

int damage_player(int damage)
{
    int adef = def;
    for(int i = 0; i < SLOT_COUNT; ++i)
        if(equipment[i])
            adef += equipment[i]->def;
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

int lua_player_newindex(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    if(!strcmp(index, "hp") && lua_isinteger(state, 3)) hp_current = lua_tointeger(state, 3);
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
