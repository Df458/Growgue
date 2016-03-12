#include "input.h"

static int last_input_type = INPUT_INVALID;
static int last_input_direction = DIRECTION_INVALID;
static int last_input_action = ACTION_INVALID;
static MEVENT mouse;

void init_input()
{
    mousemask(BUTTON3_RELEASED | BUTTON3_CLICKED, 0);
}

int get_input(WINDOW* win)
{
    last_input_type = INPUT_INVALID;

    int res;
    if(win)
        res = wgetch(win);
    else
        res = getch();
    switch(res) {
        { // DIRECTION INPUTS
        case 'h':
        case '4':
        case KEY_LEFT:
            last_input_direction = DIRECTION_WEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
            last_input_direction = DIRECTION_SOUTH;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'k':
        case '8':
        case KEY_UP:
            last_input_direction = DIRECTION_NORTH;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'l':
        case '6':
        case KEY_RIGHT:
            last_input_direction = DIRECTION_EAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'y':
        case '7':
            last_input_direction = DIRECTION_NORTHWEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'u':
        case '9':
            last_input_direction = DIRECTION_NORTHEAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'b':
        case '1':
            last_input_direction = DIRECTION_SOUTHWEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'n':
        case '3':
            last_input_direction = DIRECTION_SOUTHEAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '<':
            last_input_direction = DIRECTION_UP;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '>':
            last_input_direction = DIRECTION_DOWN;
            last_input_type = INPUT_DIRECTIONAL;
            break;
    }
    { // ACTION INPUTS

        case '\n':
            last_input_action = ACTION_SELECT;
            last_input_type = INPUT_ACTION;
            break;

        case 'q':
            last_input_action = ACTION_QUIT;
            last_input_type = INPUT_ACTION;
            break;

        case '[':
            last_input_action = ACTION_SCROLL_UP;
            last_input_type = INPUT_ACTION;
            break;

        case ']':
            last_input_action = ACTION_SCROLL_DOWN;
            last_input_type = INPUT_ACTION;
            break;
        case 't':
            last_input_action = ACTION_TILL;
            last_input_type = INPUT_ACTION;
            break;
        case 'p':
            last_input_action = ACTION_PLANT;
            last_input_type = INPUT_ACTION;
            break;
        case ',':
            last_input_action = ACTION_PICKUP;
            last_input_type = INPUT_ACTION;
            break;
        case 'd':
            last_input_action = ACTION_DROP;
            last_input_type = INPUT_ACTION;
            break;
        case 'a':
            last_input_action = ACTION_APPLY;
            last_input_type = INPUT_ACTION;
            break;
        case 'w':
            last_input_action = ACTION_WATER;
            last_input_type = INPUT_ACTION;
            break;
        case 'e':
            last_input_action = ACTION_EQUIP;
            last_input_type = INPUT_ACTION;
            break;
        case 'r':
            last_input_action = ACTION_REMOVE;
            last_input_type = INPUT_ACTION;
            break;
        case 'v':
            last_input_action = ACTION_HARVEST;
            last_input_type = INPUT_ACTION;
            break;
        case '?':
            last_input_action = ACTION_HELP;
            last_input_type = INPUT_ACTION;
            break;
        case KEY_MOUSE:
            if(getmouse(&mouse) == OK  && (mouse.bstate & BUTTON3_RELEASED || mouse.bstate & BUTTON3_CLICKED)) {
                last_input_type = INPUT_ACTION;
                last_input_action = ACTION_EXAMINE;
            } else {
                last_input_type = INPUT_INVALID;
            }
            break;
    }
    }

    return last_input_type;
}

int get_direction(WINDOW* win)
{
    int res = get_input(win);
    if(res != INPUT_DIRECTIONAL)
        return ACTION_INVALID;
    return get_last_direction();
}

int get_action(WINDOW* win)
{
    int res = get_input(win);
    if(res != INPUT_ACTION)
        return ACTION_INVALID;
    return get_last_action();
}

int get_last_input()
{
    return last_input_type;
}

int get_last_direction()
{
    return last_input_direction;
}

int get_last_action()
{
    return last_input_action;
}

void get_last_mouse_position(int* x, int* y)
{
    *x = mouse.x;
    *y = mouse.y;
}
