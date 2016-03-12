#ifndef TILE_H
#define TILE_H
#include "actor.h"
#include "item.h"
#include "plant.h"

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
    float water;
    float nutrients;
    float minerals;

    actor* actor_ref;
    plant* plant_ref;
    item** item_list;
    int item_count;
}
tile;

#endif
