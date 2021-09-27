#include "inventory.h"

static ItemSlot s_item_slots[ITEM_CATEGORY_CNT][ITEM_SLOTS_PER_CATEGORY];

void
inventory_init()
{
  inventory_clear();
}

void
inventory_clear()
{
  for (int category = 0; category < ITEM_CATEGORY_CNT; ++category)
  {
    for (int i = 0; i < ITEM_SLOTS_PER_CATEGORY; ++i)
      s_item_slots[category][i].quality = 0;
  }
}

ItemSlot
inventory_get_slot(ItemCategory category, int row, int column)
{
  return s_item_slots[category][row * INVENTORY_COLUMN_CNT + column];
}

int
inventory_add_item(ItemId id, int quality)
{
  int       added = 0;
  ItemSlot* slots = s_item_slots[item_category(id)];

  for (int i = 0; i < ITEM_SLOTS_PER_CATEGORY && quality > 0; ++i)
  {
    int space = MAX_ITEMS_PER_SLOT - slots[i].quality;
    if (slots[i].item_id == id && space >= quality)
    {
      int amount = min(space, quality);
      slots[i].quality += amount;
      quality -= amount;
      added += amount;
    }
  }
  if (quality > 0)
  {
    for (int i = 0; i < ITEM_SLOTS_PER_CATEGORY && quality > 0; ++i)
    {
      if (slots[i].quality == 0)
      {
        int amount = min(MAX_ITEMS_PER_SLOT, quality);
        added += amount;
        quality -= amount;
        slots[i].quality += amount;
        slots[i].item_id = id;
      }
    }
  }
  return added;
}

void
inventory_drop_item(ItemCategory category, int row, int column, int quality)
{
  ItemSlot* slot = &s_item_slots[category][row * INVENTORY_COLUMN_CNT + column];

  if (quality == DROP_ALL)
  {
    slot->quality = 0;
  }
  else
  {
    slot->quality = max(0, slot->quality - quality);
  }
}
