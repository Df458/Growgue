#ifdef PDCURSES
#ifdef Linux
#include <xcurses/curses.h>
#else
#include <curses.h>
#endif
#else
#include <curses.h>
#endif
#include <string.h>
#include "color.h"

int pairs[COLOR_COUNT] = { 0 };
const char* pair_names[] =
{
    "default",

    "selected",

    "HP good",
    "EP good",
    "HP low",
    "EP low",
    "HP critical",
    "EP critical",

    "Soil",
    "Plant",
    "Metal",
    "Lava",
    "Blood",
    "Carrot",
    "Growing Carrot",
    "Growing Plant",

    "White Fur",
    "White Skin",
    "Brown Fur",
    "Brown Skin",
    "Pink Skin",
    "Green Skin",

    "warning",
    "error",
};

enum low_pair_ids {
    WHITE_BLACK = 0,
    YELLOW_BLACK,
    GREEN_BLACK,
    RED_BLACK,
    BROWN_BLACK,
    BLUE_BLACK,
    PINK_BLACK,
    ORANGE_BLACK,
    ORANGE_BROWN,
    GREEN_BROWN,
};

#define COLOR_BROWN COLOR_MAGENTA
#define COLOR_PINK 8
#define COLOR_ORANGE 9

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
            init_color(COLOR_BROWN, 600, 400, 0);
            init_color(COLOR_PINK, 1000, 500, 500);
            init_color(COLOR_ORANGE, 1000, 200, 0);
        }
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
        init_pair(GREEN_BROWN, COLOR_GREEN, COLOR_BROWN);
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
        init_pair(BROWN_BLACK, COLOR_BROWN, COLOR_BLACK);
        init_pair(PINK_BLACK, COLOR_PINK, COLOR_BLACK);
        init_pair(ORANGE_BLACK, COLOR_ORANGE, COLOR_BLACK);
        init_pair(ORANGE_BROWN, COLOR_ORANGE, COLOR_BROWN);
        pairs[COLOR_SELECTION] = YELLOW_BLACK;

        pairs[COLOR_HP_GOOD]   = GREEN_BLACK;
        pairs[COLOR_HP_LOW]    = YELLOW_BLACK;
        pairs[COLOR_HP_CRIT]   = RED_BLACK;
        pairs[COLOR_EP_GOOD]   = BLUE_BLACK;
        pairs[COLOR_EP_LOW]    = YELLOW_BLACK;
        pairs[COLOR_EP_CRIT]   = RED_BLACK;

        pairs[COLOR_SOIL]      = BROWN_BLACK;
        pairs[COLOR_PLANT]     = GREEN_BLACK;
        pairs[COLOR_METAL]     = WHITE_BLACK;
        pairs[COLOR_LAVA]      = ORANGE_BLACK;
        pairs[COLOR_BLOOD]     = RED_BLACK;
        pairs[COLOR_CARROT]    = ORANGE_BLACK;
        pairs[COLOR_CARROT_PLANT] = ORANGE_BROWN;
        pairs[COLOR_SOIL_PLANT]= GREEN_BROWN;

        pairs[COLOR_WHITE_FUR] = WHITE_BLACK;
        pairs[COLOR_WHITE_SKIN] = WHITE_BLACK;
        pairs[COLOR_BROWN_FUR] = BROWN_BLACK;
        pairs[COLOR_BROWN_SKIN] = BROWN_BLACK;
        pairs[COLOR_PINK_SKIN] = PINK_BLACK;
        pairs[COLOR_GREEN_SKIN] = GREEN_BLACK;

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

const char* get_color_str(int color)
{
    return pair_names[color];
}
