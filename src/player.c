#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "input.h"
#include "log.h"
#include "player.h"

WINDOW* map_win = 0;
WINDOW* stats_win = 0;
WINDOW* hp_win = 0;

#define BAR_LENGTH 43

static int hp_max = 0;
static int ep_max = 0;
static int str = 0;
static int def = 0;

static int hp_current;
static int ep_current;

static int x = 0;
static int y = 0;

static map* current_map = 0;

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
    } else if(res == 2) {
        actor* act = get_actor_at(x + dx, y + dy, current_map);
        int dmg = damage_actor(act, str);
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

int damage_player(int damage)
{
    hp_current -= damage - def;
    return damage - def;
}
