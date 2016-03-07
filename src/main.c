#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "macro.h"
#include "menu.h"
#include "reqs.h"

enum game_state_type
{
    STATE_INVALID = -1,
    STATE_MENU,
    STATE_COUNT
};

static int game_state = STATE_MENU;

int main(int argc, char* argv[])
{
#if defined PDCURSES
    putenv("PDC_COLS=" macro_str(MIN_W));
    putenv("PDC_LINES=" macro_str(MIN_H));
    resize_term(MIN_H, MIN_W);
#endif

    srand(time(NULL));

    initscr();
    clear();
    noecho();
    cbreak();

    if(!has_colors()) {
        endwin();
        fprintf(stderr, "Error: Your terminal does not support color output.\nThis application must have color output to run.\n");
        return 1;
    }

    start_color();

    if(!can_change_color()) {
        endwin();
        fprintf(stderr, "Warning: Your terminal cannot change color definitions.\nThis may impact the game's aesthetic.\nPress any key to continue.\n");
        getch();
        refresh();
    }
    if(COLORS < MIN_COLS || COLOR_PAIRS < MIN_PAIRS) {
        endwin();
        fprintf(stderr, "Warning: Your terminal lacks sufficient color support to run this game (Expected %d colors and %d pairs, got %d colors and %d pairs).\n", MIN_COLS, MIN_PAIRS, COLORS, COLOR_PAIRS);
    }

#if !defined PDCURSES
    if(COLORS < BEST_COLS || COLOR_PAIRS < BEST_PAIRS) {
        endwin();
        fprintf(stderr, "Warning: Your terminal lacks sufficient color support to use the game's full range of color (Expected %d colors and %d pairs, got %d colors and %d pairs).\nThis may impact the game's graphics.\nPress any key to continue.\n", BEST_COLS, BEST_PAIRS, COLORS, COLOR_PAIRS);
        getch();
    }
#endif

    int rows, cols;
    max_size(stdscr, rows, cols);
    if(rows < MIN_H || cols < MIN_W) {
        endwin();
        fprintf(stderr, "This terminal is too small to play this game!\nRequired dimensions are %dx%d, but got %dx%d\n", MIN_W, MIN_H, cols, rows);
        return 1;
    }

    bool should_continue = true;

    if(!init_menu())
    {
        endwin();
        fprintf(stderr, "Failed to create menu!\n");
        return 1;
    }

    while(should_continue) {
        switch(game_state) {
            case STATE_MENU: {
                int val;
                if((val = update_menu()))
                {
                    switch(val) {
                        case SELECTION_QUIT:
                            should_continue = false;
                        default:
                            endwin();
                            fprintf(stderr, "If you see this message, something has gone terribly wrong in the menu code!\n");
                            return 1;
                    }
                }
            } break;
            default:
                endwin();
                fprintf(stderr, "If you see this message, something has gone terribly wrong in general!\n");
                return 1;
        }
    }

    endwin();
    return 0;
}
