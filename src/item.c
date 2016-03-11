#include <stdlib.h>

#include "item.h"

item* create_item(const char* path)
{
    item* it = malloc(sizeof(item));
    // TODO: Initialize the item
    return it;
}

void destroy_item(item* item)
{
    // TODO: Destroy additional data
    free(item);
}
