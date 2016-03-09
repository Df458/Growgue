#ifndef TILE_H
#define TILE_H
#include "actor.h"

typedef struct tile
{
    int display;
    int color;

    int  cost;
    bool solid;

    actor* actor_ref;
}
tile;

#endif
