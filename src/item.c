#include "item.h"
#include "equipment_system.h"

struct Sprite g_item_icons[ITEM_CNT];

const struct Item g_items[ITEM_CNT] = {
  [ITEM_RED_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Red Flask",
      .icon_name       = "flask_red",
      .description     = "restore small amount of hp",
      .stackable       = true,
      .data.consumable.hp = 5,
  },
  [ITEM_BLUE_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Blue Flask",
      .icon_name       = "flask_blue",
      .description     = "restore small amount of mp",
      .stackable       = true,
      .data.consumable.mp = 5,
  },
  [ITEM_BIG_RED_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Big Red Flask",
      .icon_name       = "flask_big_red",
      .description     = "restore large amount of hp",
      .stackable       = true,
      .data.consumable.hp = 10,
  },
  [ITEM_BIG_BLUE_FLASK] = {
      .category        = ITEM_CATEGORY_CONSUMABLE,
      .name            = "Big Blue Flask",
      .icon_name = "flask_big_blue",
      .description     = "restore large amount of mp",
      .stackable       = true,
      .data.consumable.mp = 10,
  },

  /*= SPELLS =*/
  [ITEM_FIRE_BALL] = {
      .category        = ITEM_CATEGORY_SPELL,
      .name            = "Fire Ball",
      .icon_name       = "icon_fire_ball",
      .description     = "cast fire ball",
      .stackable       = false,
  },
  [ITEM_ICE_SPIKE] = {
      .category = ITEM_CATEGORY_SPELL,
      .name = "Ice Spike",
      .icon_name = "icon_ice_arrow",
      .description = "cast ice spike",
      .stackable = false,
  },

  /*= WEAPONS  =*/
  [ITEM_ANIME_SWORD] = {
      .category = ITEM_CATEGORY_EQUIPMENT,
      .name = "Anime Sword",
      .description = "Anime Sword",
      .icon_name = "icon_weapon",
      .stackable = false,
      .data.equipment.id = EQM_ANIME_SWORD,
  },

  /*= ARMORS =*/
  [ITEM_LEATHER_ARMOR] = {
      .category = ITEM_CATEGORY_EQUIPMENT,
      .name = "Leather Armor",
      .description = "Leather Armor",
      .icon_name = "icon_leather_armor",
      .data.equipment.id = EQM_LEATHER_ARMOR,
  },

  /*= RINGS =*/
  [ITEM_SAPPHIRE_RING] = {
      .category = ITEM_CATEGORY_EQUIPMENT,
      .name = "Sapphire Rings",
      .description = "increase int",
      .icon_name = "icon_sapphire_ring",
      .data.equipment.id = EQM_SAPPHIRE_RING,
  },

  [ITEM_RUBY_RING] = {
      .category = ITEM_CATEGORY_EQUIPMENT,
      .name = "Ruby Ring",
      .description = "Rare ring made of ruby",
      .icon_name = "icon_ruby_ring",
      .data.equipment.id = EQM_RUBY_RING,
  },

  [ITEM_WOODEN_RING] = {
    .category = ITEM_CATEGORY_EQUIPMENT,
    .name = "Wooden Rings",
    .description = "increase health",
    .icon_name = "icon_wooden_ring",
    .data.equipment.id = EQM_WOODEN_RING,
  },

  /*= Boots =*/
  [ITEM_IRON_BOOTS] = {
    .category = ITEM_CATEGORY_EQUIPMENT,
    .name = "Iron Boots",
    .icon_name = "icon_iron_boots",
    .description = "Iron Boots",
    .data.equipment.id = EQM_IRON_BOOTS,
  },
};

int
item_init(const SpriteSheet* spritesheet)
{
    char          icon_name[255];
    const Sprite* icon;
    for (int i = 0; i < ITEM_CNT; ++i)
    {
        SDL_strlcpy(icon_name, g_items[i].icon_name, (sizeof icon_name) - 1);
        SDL_strlcat(icon_name, ".png", (sizeof icon_name) - 1);
        if ((icon = sprite_sheet_get(spritesheet, icon_name)) == NULL)
            LEAVE_ERROR(-1, "Item icon not found\n");
        g_item_icons[i] = *icon;
    }
    return 0;
}
