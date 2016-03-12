#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "actor.h"
#include "color.h"
#include "item.h"
#include "input.h"
#include "log.h"
#include "macro.h"
#include "map.h"
#include "menu.h"
#include "player.h"
#include "reqs.h"

enum game_state_type
{
    STATE_INVALID = -1,
    STATE_MENU,
    STATE_GAME,
    STATE_COUNT
};

static int game_state = STATE_MENU;
static bool dead = false;
map* test_map;

void init_game()
{
    WINDOW* map_win = newwin(24, 80, 0, 0);
    WINDOW* area_win = newwin(8, 10, 0, 80);
    wborder(area_win, ' ', 179, 196, 205, 196, 191, 205, 181);
    wrefresh(area_win);
    WINDOW* stats_win = newwin(8, 10, 8, 80);
    wborder(stats_win, ' ', 179, ' ', 205, ' ', 179, 205, 181);
    WINDOW* examine_win = newwin(8, 10, 16, 80);
    wborder(examine_win, ' ', 179, ' ', 205, ' ', 179, 205, 181);
    wrefresh(examine_win);
    WINDOW* hp_win = newwin(1, 90, 24, 0);
    WINDOW* log_win = newwin(10, 90, 25, 0);

    dead = false;

    init_log(log_win);
    init_map(map_win);
    init_items();
    test_map = create_map(80, 24, GEN_WALK);
    init_player(map_win, stats_win, hp_win, test_map);
    int x, y;
    get_random_empty_tile(&x, &y, test_map);
    spawn_actor(x, y, "data/test.actor", test_map);
    get_random_empty_tile(&x, &y, test_map);
    spawn_item(x, y, "data/test.item", test_map);
    get_random_empty_tile(&x, &y, test_map);
    player_set_position(x, y);
    draw_map(x, y, test_map);
    draw_log();
}

bool update_game()
{
    int in = get_input();
    if(in == INPUT_ACTION && get_last_action() == ACTION_QUIT)
        return false;
    else if(in == INPUT_ACTION && (get_last_action() == ACTION_SCROLL_UP || get_last_action() == ACTION_SCROLL_DOWN)) {
        log_scroll(get_last_action() == ACTION_SCROLL_UP);
        draw_log();
    } else if(!dead) {
        if(is_dead()) {
            add_message(COLOR_HP_CRIT, "Oh dear, you've died!");
            add_message(COLOR_DEFAULT, "Press q to return to the main menu");
            dead = true;
        } else {
            update_player();
            update_map(1, test_map);
        }
        draw_log();
    }
    return true;
}

void end_game()
{
    cleanup_items();
    destroy_map(test_map);
    cleanup_player();
    cleanup_log();
    refresh();
}

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

    if(COLORS < MIN_COLS || COLOR_PAIRS < MIN_PAIRS) {
        endwin();
        fprintf(stderr, "Warning: Your terminal lacks sufficient color support to run this game (Expected %d colors and %d pairs, got %d colors and %d pairs).\n", MIN_COLS, MIN_PAIRS, COLORS, COLOR_PAIRS);
    }

#if !defined PDCURSES
    if(COLORS < BEST_COLS || COLOR_PAIRS < BEST_PAIRS) {
        endwin();
        fprintf(stderr, "Warning: Your terminal lacks sufficient color support to use the game's full range of color (Expected %d colors and %d pairs, got %d colors and %d pairs).\nThis may impact the game's graphics.\nPress any key to continue.\n", BEST_COLS, BEST_PAIRS, COLORS, COLOR_PAIRS);
        getch();
        set_hicolor(false);
    } else {
        if(!can_change_color()) {
            endwin();
            fprintf(stderr, "Warning: Your terminal cannot change color definitions.\nThis will negatively impact the game's graphics.\nPress any key to continue.\n");
            getch();
            refresh();
            set_hicolor(false);
        } else {
            set_hicolor(true);
        }
    }
#else
    set_hicolor(false);
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
                        case SELECTION_PLAY:
                            hide_menu();
                            game_state = STATE_GAME;
                            init_game();
                            break;
                        case SELECTION_QUIT:
                            should_continue = false;
                            break;
                        default:
                            endwin();
                            fprintf(stderr, "If you see this message, something has gone terribly wrong in the menu code!\n");
                            return 1;
                    }
                }
            } break;
            case STATE_GAME:
                if(!update_game()) {
                    end_game();
                    game_state = STATE_MENU;
                    show_menu();
                }
                break;
            default:
                endwin();
                fprintf(stderr, "If you see this message, something has gone terribly wrong in general!\n");
                return 1;
        }
    }

    endwin();
    delwin(stdscr);
    return 0;
}
