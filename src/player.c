#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "player.h"

WINDOW* map_win = 0;
WINDOW* stats_win = 0;
WINDOW* hp_win = 0;

#define BAR_LENGTH 43

int hp_max = 0;
int ep_max = 0;

int hp_current;
int ep_current;

void init_stats()
{
    hp_max = 100;
    ep_max = 100;

    hp_current = hp_max;
    ep_current = ep_max;
}

void init_player(WINDOW* map, WINDOW* stats, WINDOW* hp)
{
    map_win = map;
    stats_win = stats;
    hp_win = hp;

    init_stats();

    draw_player();
}

void draw_player()
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

    wrefresh(map_win);
    wrefresh(stats_win);
    wrefresh(hp_win);
}

void update_player()
{
    draw_player();
}

void cleanup_player()
{
   delwin(map_win);
   delwin(stats_win);
   delwin(hp_win);
}
