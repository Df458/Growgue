#ifndef COLOR_H
#define COLOR_H
#ifdef PDCURSES
#ifdef Linux
#include <xcurses/curses.h>
#include <xcurses/panel.h>
#else
#include <curses.h>
#include <panel.h>
#endif
#else
#include <curses.h>
#include <panel.h>
#endif

enum pair_ids {
    COLOR_DEFAULT = 0,

    COLOR_SELECTION,

    COLOR_HP_GOOD,
    COLOR_EP_GOOD,
    COLOR_HP_LOW,
    COLOR_EP_LOW,
    COLOR_HP_CRIT,
    COLOR_EP_CRIT,

    COLOR_SOIL,
    COLOR_PLANT,
    COLOR_METAL,
    COLOR_LAVA,
    COLOR_BLOOD,
    COLOR_CARROT,
    COLOR_CARROT_PLANT,
    COLOR_SOIL_PLANT,

    COLOR_WHITE_FUR,
    COLOR_WHITE_SKIN,
    COLOR_BROWN_FUR,
    COLOR_BROWN_SKIN,
    COLOR_PINK_SKIN,
    COLOR_GREEN_SKIN,

    COLOR_WARNING,
    COLOR_FATAL,
    COLOR_COUNT
};

void set_hicolor(bool hi);
void set_color(WINDOW* win, int id);
int color_str(const char* str);
const char* get_color_str(int color);

#endif
