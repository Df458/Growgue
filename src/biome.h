#ifndef BIOME_H
#define BIOME_H
#include "tile.h"

typedef struct biome
{
    int* floor_chars;
    int floor_count;
    int* wall_chars;
    int wall_count;

    float water_min;
    float water_max;
    float nutrients_min;
    float nutrients_max;
    float minerals_min;
    float minerals_max;
}
biome;

biome* create_biome(const char* file);
void destroy_biome(biome* b);
void apply_biome(tile* t, biome* b);

#endif
