#include <curses.h>
#include <panel.h>
#include "draw.h"
#include "input.h"
#include "menu.h"
#include "reqs.h"
#include "color.h"

static WINDOW* menu_window = 0;
static PANEL* menu_panel   = 0;
static int current_selection = SELECTION_NONE;
static const char* menu_entries[SELECTION_COUNT - 1] = {
    "Start Game",
    "Instructions",
    "Quit",
};

bool init_menu()
{
    if(menu_window || menu_panel)
        return false;

    menu_window = newwin(MIN_H, MIN_W, 0, 0);
    menu_panel  = new_panel(menu_window);

    if(!menu_window || !menu_panel)
        return false;

    return true;
}

void draw_menu()
{
    draw_text_aligned(menu_window, 4, "Growgue", ALIGN_CENTER);

    for(int i = 0; i < SELECTION_COUNT - 1; ++i) {
        /* if(i == current_selection - 1) */
        /*     set_color(MENU_COLOR); */
        draw_text_aligned(menu_window, 6 + i, menu_entries[i], ALIGN_CENTER);
    }

    update_panels();
    doupdate();
}

int update_menu()
{
    draw_menu();
    int dir = get_direction();
    if(dir == DIRECTION_NORTH) {
        // TODO: Decrement selection
    } else if(dir == DIRECTION_SOUTH) {
        // TODO: Increment selection
    } else if(dir == DIRECTION_INVALID) {
        if(get_last_input() == INPUT_ACTION) {
            int act = get_last_action();
            if(act == ACTION_SELECT)
                return current_selection;
        }
    }

    draw_menu();

    return SELECTION_NONE;
}
