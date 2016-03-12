#include <curses.h>
#include <panel.h>
#include "color.h"
#include "controls.h"
#include "input.h"
#include "reqs.h"

static int help_length = 26;
static const char* help_text[] = 
{
    "Movement",
    "y  k  u | 7  8  9",
    " \\ | /  |  \\ | /",
    "h-   -l | 4-   -6",
    " / | \\  |  / | \\",
    "b  j  n | 1  2  3",
    "",   
    "Items",
    ",: Pick up an item",
    "d: Drop an item",
    "a: Apply(Use) and item",
    "e: Equip an item",
    "r: Remove an item",
    "",
    "Farming",
    "t: Till soil",
    "w: Water soil/plants",
    "p: Plant an item",
    "v: Harvest an item",
    "",
    "Misc",
    "[/]: Scroll backwards/forwards",
    "enter: Confirm/Select",
    "q: Quit/Close/Cancel",
    "y/n Yes/No",
    "?: View this help page"
};

void show_controls()
{
    int rows, cols;
    max_size(stdscr, rows, cols);

    WINDOW* ctrl_window = newwin(rows, cols, 0, 0);
    PANEL* ctrl_panel   = new_panel(ctrl_window);
    show_panel(ctrl_panel);

    bool should_continue = true;
    while(should_continue) {
        set_color(ctrl_window, COLOR_DEFAULT);
        wborder(ctrl_window, 179, 179, 196, 196, 218, 191, 192, 217);
        for(int i = 0; i < help_length; ++i) {
            mvwaddstr(ctrl_window, i + 1, 1, help_text[i]);
        }
        update_panels();
        doupdate();

        int res = get_action();
        switch(res) {
            case ACTION_QUIT:
                should_continue = false;
                break;
        }
    }

    hide_panel(ctrl_panel);
    update_panels();
    doupdate();

    del_panel(ctrl_panel);
    delwin(ctrl_window);
}
