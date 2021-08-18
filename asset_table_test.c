#include "graphics/asset_table.h"
#include <stdio.h>

int main()
{
  AssetTable tbl;

  asset_table_init(&tbl, NULL);

  asset_table_insert(&tbl, "key1", "value1");
  asset_table_insert(&tbl, "key2", "value2");
  asset_table_insert(&tbl, "key3", "value3");
  asset_table_insert(&tbl, "key4", "value4");
  asset_table_insert(&tbl, "key5", "value5");
  asset_table_insert(&tbl, "key6", "value6");
  asset_table_insert(&tbl, "key7", "value7");
  asset_table_insert(&tbl, "key8", "value8");
  asset_table_insert(&tbl, "key7", "value9");


  asset_table_lookup(&tbl, "key5");


  asset_table_destroy(&tbl);
}
