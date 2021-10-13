#include "inventory.h"

static struct ItemSlot s_item_slots[ITEM_CATEGORY_CNT][ITEM_SLOTS_PER_CATEGORY];

#define GET_ITEM_SLOT(cat, row, col) (&s_item_slots[cat][row * INVENTORY_COLUMN_CNT + col])

void
inv_init()
{
    inv_clear();
}

void
inv_clear()
{
    for (int category = 0; category < ITEM_CATEGORY_CNT; ++category)
    {
        for (int i = 0; i < ITEM_SLOTS_PER_CATEGORY; ++i)
            s_item_slots[category][i].quantity = 0;
    }
}

struct ItemSlot
inv_get_slot(enum ItemCategory category, int row, int column)
{
    return *GET_ITEM_SLOT(category, row, column);
}

int
inv_add_item(enum ItemId id, int quantity)
{
    int              added   = 0;
    struct ItemSlot* slots   = s_item_slots[item_category(id)];
    const int        n_slots = ITEM_SLOTS_PER_CATEGORY;

    for (int i = 0; i < n_slots && quantity > 0; ++i)
    {
        int space = MAX_ITEMS_PER_SLOT - slots[i].quantity;
        if (slots[i].item_id == id && space >= quantity)
        {
            int amount = min(space, quantity);
            slots[i].quantity += amount;
            quantity -= amount;
            added += amount;
        }
    }

    // add remaining item to new slot
    if (quantity > 0)
    {
        for (int i = 0; i < n_slots && quantity > 0; ++i)
        {
            if (slots[i].quantity == 0)
            {
                int amount = min(MAX_ITEMS_PER_SLOT, quantity);
                added += amount;
                quantity -= amount;
                slots[i].quantity += amount;
                slots[i].item_id = id;
            }
        }
    }
    return added;
}

void
inv_drop_item(enum ItemCategory category, int row, int column, int quantity)
{
    struct ItemSlot* slot = GET_ITEM_SLOT(category, row, column);

    if (quantity == DROP_ALL)
    {
        slot->quantity = 0;
    }
    else
    {
        slot->quantity = max(0, slot->quantity - quantity);
    }
}
