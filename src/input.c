#include <curses.h>
#include "input.h"

static int last_input_type = INPUT_INVALID;
static int last_input_direction = DIRECTION_INVALID;
static int last_input_action = ACTION_INVALID;

int get_input()
{
    last_input_type = INPUT_INVALID;

    int res = getch();
    switch(res) {
        case 'h':
        case '4':
            last_input_direction = DIRECTION_WEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'j':
        case '2':
            last_input_direction = DIRECTION_SOUTH;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'k':
        case '8':
            last_input_direction = DIRECTION_NORTH;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case 'l':
        case '6':
            last_input_direction = DIRECTION_EAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '7':
            last_input_direction = DIRECTION_NORTHWEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '9':
            last_input_direction = DIRECTION_NORTHEAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '1':
            last_input_direction = DIRECTION_SOUTHWEST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '3':
            last_input_direction = DIRECTION_SOUTHEAST;
            last_input_type = INPUT_DIRECTIONAL;
            break;

        case '\n':
            last_input_action = ACTION_SELECT;
            last_input_type = INPUT_ACTION;
            break;
    }

    return last_input_type;
}

int get_direction()
{
    int res = get_input();
    if(res != INPUT_DIRECTIONAL)
        return ACTION_INVALID;
    return get_last_direction();
}

int get_action()
{
    int res = get_input();
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
