// ngotrung Sat 04 Sep 2021 04:36:05 PM +07
#ifndef INVENTORY_H
#define INVENTORY_H
#include "item.h"

#define INVENTORY_COLUMN_CNT 7
#define INVENTORY_ROW_CNT 3
#define ITEM_SLOTS_PER_CATEGORY (INVENTORY_ROW_CNT * INVENTORY_COLUMN_CNT)
#define DROP_ALL (-1)
#define MAX_ITEMS_PER_SLOT 99

struct ItemSlot
{
    enum ItemId item_id;
    int         quantity;
};

void inv_init(void);

struct ItemSlot inv_get_slot(enum ItemCategory category, int row, int column);

/* return number of items was added */
int inv_add_item(enum ItemId id, int quality);

void inv_clear(void);

void inv_drop_item(enum ItemCategory category, int row, int column, int quality);

bool inv_has_item(enum ItemId id);

#endif // INVENTORY_H
