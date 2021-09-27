// ngotrung Sat 04 Sep 2021 04:36:05 PM +07
#ifndef INVENTORY_H
#define INVENTORY_H
#include "item.h"

#define INVENTORY_COLUMN_CNT 7
#define INVENTORY_ROW_CNT 3
#define ITEM_SLOTS_PER_CATEGORY (INVENTORY_ROW_CNT * INVENTORY_COLUMN_CNT)
#define DROP_ALL (-1)
#define MAX_ITEMS_PER_SLOT 64

typedef struct ItemSlot
{
  enum ItemId item_id;
  int         quality;
} ItemSlot;

void inventory_init(void);

ItemSlot inventory_get_slot(ItemCategory category, int row, int column);

/* return number of items was added */
int inventory_add_item(ItemId id, int quality);

void inventory_clear(void);

void inventory_drop_item(ItemCategory category, int row, int column, int quality);

bool inventory_has_item(ItemId id);



#endif // INVENTORY_H
