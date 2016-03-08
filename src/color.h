#ifndef COLOR_H
#define COLOR_H
#include <stdbool.h>
#include <curses.h>

enum pair_ids {
    COLOR_DEFAULT = 0,

    COLOR_SELECTION,

    COLOR_HP_GOOD,
    COLOR_EP_GOOD,
    COLOR_HP_LOW,
    COLOR_EP_LOW,
    COLOR_HP_CRIT,
    COLOR_EP_CRIT,

    COLOR_WARNING,
    COLOR_FATAL,
    COLOR_COUNT
};

void set_hicolor(bool hi);
void set_color(WINDOW* win, int id);

#endif
