// ngotrung Sat 04 Sep 2021 05:49:44 PM +07
#ifndef RESOURCES_H
#define RESOURCES_H
#include "graphics.h"

typedef struct Resources
{
    SpriteSheet spritesheet;
    Font        font_16px;
    Font        font_8px;
} Resources;

int  load_resources(Resources* resources);
void release_resources(Resources* resources);

#endif // RESOURCES_H
