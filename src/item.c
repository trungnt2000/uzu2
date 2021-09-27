#include "item.h"

static Sprite s_item_icons[ITEM_CNT];

static const char s_item_icon_file_names[ITEM_CNT][64] = {
    [ITEM_RED_FLASK] = "flask_red.png",         [ITEM_BLUE_FLASK] = "flask_blue.png",
    [ITEM_BIG_RED_FLASK] = "flask_big_red.png", [ITEM_BIG_BLUE_FLASK] = "flask_big_blue.png",
    [ITEM_FIRE_BALL] = "icon_fire_ball.png",    [ITEM_ICE_SPIKE] = "icon_ice_arrow.png",
};

#define ICON(Id) (&s_item_icons[ITEM_##Id])

const Item g_items[ITEM_CNT] = {
  [ITEM_RED_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Red Flask",
      .description     = "restore small amount of hp",
      .icon            = ICON(RED_FLASK),
      .stackable       = true,
      .data.consumable = { .hp = 5 },
  },
  [ITEM_BLUE_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Blue Flask",
      .description     = "restore small amount of mp",
      .icon            = ICON(BLUE_FLASK),
      .stackable       = true,
      .data.consumable = { .mp = 5 },
  },
  [ITEM_BIG_RED_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Big Red Flask",
      .description     = "restore large amount of hp",
      .icon            = ICON(BIG_RED_FLASK),
      .stackable       = true,
      .data.consumable = { .hp = 10 },
  },
  [ITEM_BIG_BLUE_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Big Blue Flask",
      .description     = "restore large amount of mp",
      .icon            = ICON(BIG_BLUE_FLASK),
      .stackable       = true,
      .data.consumable = { .mp = 10 },
  },
  [ITEM_FIRE_BALL] = {
      .category        = ITEM_CATEGORY_EQUIPMENT,
      .name            = "Fire Ball",
      .description     = "cast fire ball",
      .icon            = ICON(BIG_BLUE_FLASK),
      .stackable       = false,
      .data.spell = { 0 },
  },
  [ITEM_ICE_SPIKE] = {
      .category = ITEM_CATEGORY_SPELL,
      .name = "Ice Spike",
      .description = "cast ice spike",
      .icon = ICON(ICE_SPIKE),
      .stackable = false,
      .data.spell = { 0 },
  },
};

int
item_init(const SpriteSheet* spritesheet)
{
    const Sprite* icon;
    for (int i = 0; i < ITEM_CNT; ++i)
    {
        if ((icon = sprite_sheet_get(spritesheet, s_item_icon_file_names[i])) == NULL)
            LEAVE_ERROR(-1, "Item icon not found\n");

        s_item_icons[i] = *icon;
    }
    return 0;
}
