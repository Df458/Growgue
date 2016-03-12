#ifndef INPUT_H
#define INPUT_H

enum input_types {
    INPUT_INVALID = -1,
    INPUT_DIRECTIONAL,
    INPUT_ACTION,
    INPUT_COUNT
};

enum input_directions {
    DIRECTION_INVALID = -1,

    DIRECTION_NORTH,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST,

    DIRECTION_NORTHEAST,
    DIRECTION_NORTHWEST,
    DIRECTION_SOUTHEAST,
    DIRECTION_SOUTHWEST,

    DIRECTION_UP,
    DIRECTION_DOWN,

    DIRECTION_COUNT
};

enum input_actions {
    ACTION_INVALID = -1,
    ACTION_SELECT,
    ACTION_QUIT,
    ACTION_SCROLL_UP,
    ACTION_SCROLL_DOWN,
    ACTION_TILL,
    ACTION_WATER,
    ACTION_PLANT,
    ACTION_PICKUP,
    ACTION_DROP,
    ACTION_APPLY,
    ACTION_EQUIP,
    ACTION_REMOVE,
    ACTION_HARVEST,
    ACTION_HELP,
    ACTION_COUNT,
};

int get_input();
int get_direction();
int get_action();

int get_last_input();
int get_last_direction();
int get_last_action();

#endif
