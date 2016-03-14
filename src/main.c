#define true TRUE
#define false FALSE
#ifdef PDCURSES
#ifdef Linux
#include <xcurses/curses.h>
#else
#include <curses.h>
#endif
#else
#include <curses.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "actor.h"
#include "color.h"
#include "controls.h"
#include "item.h"
#include "input.h"
#include "log.h"
#include "luafunc.h"
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
    WINDOW* area_win = newwin(6, 10, 0, 80);
    WINDOW* stats_win = newwin(8, 10, 6, 80);
    WINDOW* examine_win = newwin(10, 10, 14, 80);
    WINDOW* hp_win = newwin(1, 90, 24, 0);
    WINDOW* log_win = newwin(10, 90, 25, 0);

    dead = false;

    init_log(log_win);
    init_map(map_win);
    init_items();
    world = calloc(LEVEL_COUNT, sizeof(map*));
    world[0] = load_map("data/maps/farm.map", 78, 22, false, true);
    for(int i = 1; i < 3; ++i)
        world[i] = load_map("data/maps/easy_cave.map", 80 + 10 * (i - 1), 24 + 4 * (i - 1), true, true);
    for(int i = 3; i < 5; ++i)
        world[i] = load_map("data/maps/mid_cave.map", 80 + 10 * (i - 1), 24 + 4 * (i - 1), true, true);
    for(int i = 5; i < 7; ++i)
        world[i] = load_map("data/maps/hard_cave.map", 80 + 15 * (i - 1), 24 + 6 * (i - 1), true, true);
    for(int i =7; i < 9; ++i)
        world[i] = load_map("data/maps/crazy_cave.map", 80 + 18 * (i - 1), 24 + 8 * (i - 1), true, true);
    world[LEVEL_COUNT - 1] = load_map("data/maps/final.map", 78, 22, true, false);
    init_player(map_win, stats_win, hp_win, area_win, examine_win, world[0]);
    int x, y;
    get_random_empty_tile(&x, &y, world[LEVEL_COUNT - 1]);
    spawn_item(x, y, "data/items/cat.item", world[LEVEL_COUNT - 1]);

    get_random_empty_tile(&x, &y, world[0]);
    player_set_position(x, y);
    draw_map(x, y, world[0]);
    add_message(COLOR_DEFAULT, "@ symbol, a brave young farmer, was out for a stroll on his farm when his cat Cuddles ran down into the gaping starcase to the deadly Caves of Consternation! @ symbol had been meaning to patch that up for a while, but hadn't gotten a chance yet. Don't judge.");
    if(ask_question(COLOR_SELECTION, "Will you help @ symbol retrieve his cat, Cuddles?"))
        add_message(COLOR_HP_GOOD, "Excellent! Get to it, then!");
    else {
        add_message(COLOR_HP_CRIT, "Well that was unexpected. Okay then, press q to quit to the main menu.");
        dead = true;
    }
    draw_log();
}

bool update_game()
{
    int in = get_input(get_map_window());
    if(in == INPUT_ACTION && get_last_action() == ACTION_QUIT)
        return false;
    else if(in == INPUT_ACTION && (get_last_action() == ACTION_SCROLL_UP || get_last_action() == ACTION_SCROLL_DOWN)) {
        log_scroll(get_last_action() == ACTION_SCROLL_UP);
        draw_log();
    } else if(!dead) {
        update_player();
        if(get_current_floor() != current_level) {
            set_current_map(world[get_current_floor()], current_level < get_current_floor(), current_level > get_current_floor());
            current_level = get_current_floor();
        }
        update_map(1, world[current_level]);
        if(is_dead()) {
            add_message(COLOR_HP_CRIT, "Oh dear, you've died!");
            add_message(COLOR_DEFAULT, "Press q to return to the main menu");
            dead = true;
        }
        if(game_won())
            dead = true;
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

#ifdef Linux
    if(COLORS < MIN_COLS || COLOR_PAIRS < MIN_PAIRS) {
        endwin();
        fprintf(stderr, "Warning: Your terminal lacks sufficient color support to run this game (Expected %d colors and %d pairs, got %d colors and %d pairs).\n", MIN_COLS, MIN_PAIRS, COLORS, COLOR_PAIRS);
    }
#endif

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

    init_input();

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
