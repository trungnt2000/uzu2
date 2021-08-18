// ngotrung Tue 10 Aug 2021 09:50:49 AM +07
#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "toolbox/common.h"
#include <stdbool.h>
#include <stdlib.h>

#define ASSET_KEY_SIZE 256
typedef char asset_key_t[ASSET_KEY_SIZE];

typedef struct AssetTable
{
  asset_key_t* keys;
  u32*         hashes;
  void**       values;
  u32          size;
  u32          tombstoneCnt;
  u32          tombstoneCntThreshold;
  u32          usedCnt;
  u32          usedCntThreshold;
} AssetTable;

void asset_table_init(AssetTable* table);

void asset_table_destroy(AssetTable* table);

/**
 * \brief lookup entry
 *
 * \return index of given key
 */
void* asset_table_lookup(const AssetTable* table, const asset_key_t key);

/**
 * \brief erase an entry
 * \return deleted value
 */
void* asset_table_erase(AssetTable* table, const asset_key_t key);

/**
 * \brief insert an entry
 */
void asset_table_insert(AssetTable* table, const asset_key_t key, void* value);

u32 asset_table_count(AssetTable* table);

#endif // ASSET_MANAGER_H
