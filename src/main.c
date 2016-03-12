#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "actor.h"
#include "color.h"
#include "controls.h"
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
static map** world;
static int current_level = 0;

void init_game()
{
    WINDOW* map_win = newwin(24, 80, 0, 0);
    WINDOW* area_win = newwin(8, 10, 0, 80);
    WINDOW* stats_win = newwin(8, 10, 8, 80);
    WINDOW* examine_win = newwin(8, 10, 16, 80);
    WINDOW* hp_win = newwin(1, 90, 24, 0);
    WINDOW* log_win = newwin(10, 90, 25, 0);

    dead = false;

    init_log(log_win);
    init_map(map_win);
    init_items();
    /* test_map = create_map(80, 24, GEN_WALK); */
    world = calloc(LEVEL_COUNT, sizeof(map*));
    world[0] = create_map(78, 22, GEN_WALK, false, true);
    for(int i = 1; i < LEVEL_COUNT - 1; ++i)
        world[i] = create_map(80 + 10 * (i - 1), 24 + 4 * (i - 1), GEN_WALK, true, true);
    world[LEVEL_COUNT - 1] = create_map(78, 22, GEN_WALK, true, false);
    init_player(map_win, stats_win, hp_win, area_win, examine_win, world[0]);
    int x, y;
    get_random_empty_tile(&x, &y, world[0]);
    spawn_actor(x, y, "data/test.actor", world[0]);
    get_random_empty_tile(&x, &y, world[0]);
    spawn_item(x, y, "data/test.item", world[0]);
    get_random_empty_tile(&x, &y, world[0]);
    player_set_position(x, y);
    draw_map(x, y, world[0]);
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
            if(get_current_floor() != current_level) {
                set_current_map(world[get_current_floor()], current_level < get_current_floor(), current_level > get_current_floor());
                current_level = get_current_floor();
            }
            update_map(1, world[current_level]);
        }
        draw_log();
    }
    return true;
}

void end_game()
{
    move(0, 0);
    cleanup_items();
    for(int i = 0; i < LEVEL_COUNT; ++i)
        destroy_map(world[i]);
    free(world);
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
                        case SELECTION_CONTROLS:
                            show_controls();
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
