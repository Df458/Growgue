#ifndef TILE_H
#define TILE_H
#include "actor.h"

typedef struct tile
{
    int display;
    int color;

    int  cost;
    int  dist;
    bool checked;
    bool added;
    bool solid;

    bool can_till;
    bool tilled;

    actor* actor_ref;
}
tile;

#endif
