#ifndef ITEM_H
#define ITEM_H
#include "ecs.h"
#include "graphics.h"

typedef enum ItemId
{
  ITEM_RED_FLASK,
  ITEM_BLUE_FLASK,
  ITEM_BIG_RED_FLASK,
  ITEM_BIG_BLUE_FLASK,

  ITEM_FIRE_BALL,
  ITEM_ICE_SPIKE,
  ITEM_CNT
} ItemId;

typedef enum ItemCategory
{
  ITEM_CATEGORY_CONSUMABLE,
  ITEM_CATEGORY_EQUIPMENT,
  ITEM_CATEGORY_SPELL,
  ITEM_CATEGORY_KEY,
  ITEM_CATEGORY_CNT
} ItemCategory;

struct ItemConsumable
{
  int hp; // amount hit points to heal
  int mp; // amount mana points to heal
};

struct ItemEquipment
{
  int weapon_id;
};

struct ItemSpell
{
  int spell_id;
};

struct ItemKey
{
  int key_id;
};

typedef union ItemData
{
  struct ItemConsumable consumable;
  struct ItemEquipment  equipment;
  struct ItemSpell      spell;
  struct ItemKey        key;
} ItemData;

typedef struct Item
{
  const char*       name;
  const char*       description;
  ItemData          data;
  const Sprite*     icon;
  enum ItemCategory category;
  bool              stackable;
} Item;

typedef struct ItemStock
{
  enum ItemId item_id;
  u32         price;
  u32         quality;
} ItemStock;

/* load item icons */
int item_init(const SpriteSheet* spritesheet);

#define item_stackable(id) (g_items[id].stackable)
#define item_icon(id) (g_items[id].icon)
#define item_data(id) (g_items[id].data)
#define item_category(id) (g_items[id].category)
#define item_description(id) (g_items[id].description)
#define item_name(id) (g_items[id].name)

extern const Item g_items[ITEM_CNT];
#endif
