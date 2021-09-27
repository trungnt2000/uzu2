#include "resources.h"
#include "toolbox.h"

int
load_resources(Resources* resources)
{
    if (sprite_sheet_load(&resources->spritesheet, "res/spritesheet.json") != 0)
        LEAVE_ERROR(-1, "Unable to load spritesheet\n");

    if (font_load(&resources->font_16px, "res/font/font.TTF", 16) != 0)
        LEAVE_ERROR(-1, "Unable to load font\n");

    if (font_load(&resources->font_8px, "res/font/font.TTF", 8) != 0)
        LEAVE_ERROR(-1, "Unable to load font\n");

    return 0;
}

void
release_resources(Resources* resource)
{
    sprite_sheet_destroy(&resource->spritesheet);
    font_destroy(&resource->font_16px);
    font_destroy(&resource->font_8px);
}
