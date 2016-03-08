#ifndef MENU_H
#define MENU_H
#include <stdbool.h>

enum menu_selection {
    SELECTION_INVALID = -1,
    SELECTION_NONE,
    SELECTION_PLAY,
    SELECTION_CONTROLS,
    SELECTION_QUIT,
    SELECTION_COUNT
};

bool init_menu();
int update_menu();
void draw_menu();
void hide_menu();
void show_menu();

#endif
