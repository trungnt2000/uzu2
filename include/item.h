#ifndef ITEM_H
#define ITEM_H
#include "ecs.h"
#include "global.h"
#include "graphics.h"

enum ItemCategory
{
    ITEM_CATEGORY_CONSUMABLE,
    ITEM_CATEGORY_EQUIPMENT,
    ITEM_CATEGORY_SPELL,
    ITEM_CATEGORY_KEY,
    ITEM_CATEGORY_CNT
};

struct ItemConsumable
{
    s16 hp;
    s16 mp;
};

struct ItemEquipment
{
    u32 id;
};

struct ItemSpell
{
    u32 id;
};

struct ItemKey
{
    u32 id;
};

union ItemData
{
    struct ItemConsumable consumable;
    struct ItemEquipment  equipment;
    struct ItemSpell      spell;
    struct ItemKey        key;
};

struct Item
{
    const char*       name;
    const char*       description;
    const char*       icon_name;
    union ItemData    data;
    const Sprite*     icon;
    enum ItemCategory category;
    bool              stackable;
};

struct ItemStock
{
    u32 item_id;
    u32 price;
    u32 quantity;
};

/* load item icons */
int item_init(const SpriteSheet* spritesheet);

#define item_stackable(id) (g_items[id].stackable)
#define item_icon(id) ((const struct Sprite*)&g_item_icons[id])
#define item_data(id) (g_items[id].data)
#define item_category(id) (g_items[id].category)
#define item_description(id) (g_items[id].description)
#define item_name(id) (g_items[id].name)

extern const struct Item g_items[ITEM_CNT];
extern struct Sprite     g_item_icons[ITEM_CNT];
#endif
