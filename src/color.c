#include <curses.h>
#include <string.h>
#include "color.h"

int pairs[COLOR_COUNT] = { 0 };
const char* pair_names[] =
{
    "default",

    "selected",

    "hp good",
    "EP good",
    "HP low",
    "EP low",
    "HP critical",
    "EP critical",

    "warning",
    "error",
};

enum low_pair_ids {
    WHITE_BLACK = 0,
    YELLOW_BLACK,
    GREEN_BLACK,
    RED_BLACK,
    BLUE_BLACK
};

void set_hicolor(bool hi)
{
    if(hi) {
        for(int i = 0; i < COLOR_COUNT; ++i) {
            pairs[i] = i;
        }
        init_pair(COLOR_SELECTION, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_HP_GOOD, COLOR_GREEN, COLOR_BLACK); 
        init_pair(COLOR_EP_GOOD, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_HP_LOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_EP_LOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_HP_CRIT, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_EP_CRIT, COLOR_RED, COLOR_BLACK);
    } else {
        if(can_change_color()) {
            init_color(COLOR_YELLOW, 1000, 1000, 0);
            init_color(COLOR_BLUE, 0, 0, 1000);
            init_color(COLOR_RED, 1000, 0, 0);
        }
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
        pairs[COLOR_SELECTION] = YELLOW_BLACK;
        pairs[COLOR_HP_GOOD]   = GREEN_BLACK;
        pairs[COLOR_HP_LOW]    = YELLOW_BLACK;
        pairs[COLOR_HP_CRIT]   = RED_BLACK;
        pairs[COLOR_EP_GOOD]   = BLUE_BLACK;
        pairs[COLOR_EP_LOW]    = YELLOW_BLACK;
        pairs[COLOR_EP_CRIT]   = RED_BLACK;
        pairs[COLOR_WARNING]   = YELLOW_BLACK;
        pairs[COLOR_FATAL]     = RED_BLACK;
    }
}

void set_color(WINDOW* win, int id)
{
    wcolor_set(win, pairs[id], 0);
}

int color_str(const char* str)
{
    for(int i = 0; i < COLOR_COUNT; ++i)
        if(!strcmp(str, pair_names[i]))
            return i;
    return 0;
}
