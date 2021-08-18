#include "graphics/asset_table.h"

#define TOMBSTONE 1
#define LOAD_FACTOR 0.75
#define EMPTY 0

#define HASH_IS_REAL(x) ((x) != EMPTY && (x) != TOMBSTONE)
#define IS_DELETED_SLOT(x) ((x) == TOMBSTONE)
#define IS_EMPTY_SLOT(x) ((x) == EMPTY)
#define IS_USED_SLOT(x) (HASH_IS_REAL(x))

static u32
jenkins_one_at_a_time_hash(const char* key)
{
  u32       hash = 13;
  const u8* iter = (const u8*)key;
  while (*iter)
  {
    hash += *iter++;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

static u32
asset_key_hash(const asset_key_t key)
{
  return jenkins_one_at_a_time_hash(key);
}

static bool
asset_key_equals(const asset_key_t lhs, const asset_key_t rhs)
{
  return SDL_strncmp(lhs, rhs, sizeof(asset_key_t) - 1) == 0;
}

static void
asset_key_copy(asset_key_t dst, const asset_key_t src)
{
  SDL_strlcpy(dst, src, sizeof(asset_key_t) - 1);
}

static bool
asset_table_set_impl(asset_key_t*      keys,
                     u32*              hashes,
                     void**            values,
                     u32               hashValue,
                     const asset_key_t key,
                     void*             value,
                     u32               size)
{
  const u32 mask           = size - 1;
  u32       index          = hashValue & mask;
  bool      hasDeletedSlot = false;
  u32       firstDeletedSlot;
  u32       x = 0;

  // probing until find empty slot
  // or find slot which has same key
  while (!IS_EMPTY_SLOT(hashes[index]))
  {
    if (IS_DELETED_SLOT(hashes[index]) && !hasDeletedSlot)
    {
      hasDeletedSlot   = true;
      firstDeletedSlot = index;
    }
    else if (hashes[index] == hashValue && asset_key_equals(keys[index], key))
    {
      asset_key_copy(keys[index], key);
      values[index] = value;
      hashes[index] = hashValue;
      return false;
    }
    x++;
    index = (index + x) & mask;
  }
  if (hasDeletedSlot)
    index = firstDeletedSlot;

  asset_key_copy(keys[index], key);
  values[index] = value;
  hashes[index] = hashValue;
  return true;
}

static void
asset_table_rehash(AssetTable* tbl, u32 size)
{
  u32*         hashes = SDL_calloc(size, sizeof *hashes);
  asset_key_t* keys   = SDL_calloc(size, sizeof *keys);
  void**       values = SDL_calloc(size, sizeof *values);


  for (u32 i = 0; i < tbl->size; ++i)
  {
    if (IS_USED_SLOT(tbl->hashes[i]))
    {
      asset_table_set_impl(keys, hashes, values, tbl->hashes[i], tbl->keys[i], tbl->values[i], size);
    }
  }
  SDL_free(tbl->keys);
  SDL_free(tbl->hashes);
  SDL_free(tbl->values);

  tbl->hashes = hashes;
  tbl->keys   = keys;
  tbl->values = values;

  tbl->tombstoneCntThreshold = size * 3 / 16;
  tbl->usedCntThreshold      = size * 12 / 16;
  tbl->size                  = size;
  tbl->tombstoneCnt          = 0;
}

void
asset_table_init(AssetTable* tbl, FreeFunc freeFunc)
{
  tbl->size                  = 16;
  tbl->keys                  = SDL_calloc(tbl->size, sizeof *tbl->keys);
  tbl->hashes                = SDL_calloc(tbl->size, sizeof *tbl->hashes);
  tbl->values                = SDL_calloc(tbl->size, sizeof *tbl->values);
  tbl->tombstoneCnt          = 0;
  tbl->tombstoneCntThreshold = tbl->size * 3 / 16;
  tbl->usedCnt               = 0;
  tbl->usedCntThreshold      = tbl->size * 12 / 16;
  tbl->freeFunc              = freeFunc;
}

void
asset_table_destroy(AssetTable* tbl)
{
  if (tbl == NULL)
      return;
  if (tbl->freeFunc && tbl->hashes)
  {
    for (u32 i = 0; i < tbl->size; ++i)
    {
      if (IS_USED_SLOT(tbl->hashes[i]))
      {
        tbl->freeFunc(tbl->values[i]);
      }
    }
  }
  SDL_free(tbl->keys);
  SDL_free(tbl->values);
  SDL_free(tbl->hashes);
  tbl->freeFunc = NULL;
  tbl->hashes = NULL;
  tbl->keys = NULL;
  tbl->values = NULL;
}

void
asset_table_insert(AssetTable* tbl, const asset_key_t key, void* value)
{
  const u32 hashValue = asset_key_hash(key);
  ASSERT(hashValue != TOMBSTONE && hashValue != EMPTY);
  if (asset_table_set_impl(tbl->keys, tbl->hashes, tbl->values, hashValue, key, value, tbl->size))
  {
    tbl->usedCnt++;
    if (tbl->usedCnt > tbl->usedCntThreshold)
    {
      asset_table_rehash(tbl, tbl->size * 2);
    }
  }
}

void*
asset_table_lookup(const AssetTable* tbl, const asset_key_t key)
{
  const u32          mask      = tbl->size - 1;
  const u32*         hashes    = tbl->hashes;
  const asset_key_t* keys      = tbl->keys;
  const u32          hashValue = asset_key_hash(key);
  u32                idx       = hashValue & mask;
  u32                x         = 0;

  ASSERT(hashValue != TOMBSTONE && hashValue != EMPTY);
  while (!IS_EMPTY_SLOT(hashes[idx]))
  {
    if (hashes[idx] == hashValue && asset_key_equals(keys[idx], key))
    {
      return tbl->values[idx];
    }

    x++;
    idx = (idx + x) & mask;
  }
  return NULL;
}

void*
asset_table_steal(AssetTable* tbl, const asset_key_t key)
{
  const u32          hashValue = asset_key_hash(key);
  const u32          size      = tbl->size;
  u32                x         = 0;
  u32*               hashes    = tbl->hashes;
  const asset_key_t* keys      = tbl->keys;
  void*              retVal;
  const u32          mask = size - 1;
  u32                idx  = hashValue & mask;

  while (!IS_EMPTY_SLOT(hashes[idx]))
  {
    if (IS_USED_SLOT(hashes[idx]) && hashes[idx] == hashValue && asset_key_equals(keys[idx], key))
    {
      hashes[idx] = TOMBSTONE;
      tbl->tombstoneCnt++;
      tbl->usedCnt--;
      retVal = tbl->values[idx];
      if (tbl->tombstoneCnt > tbl->tombstoneCntThreshold)
        asset_table_rehash(tbl, tbl->size);
      return retVal;
    }
    x++;
    idx = (idx + x) & mask;
  }
  return NULL;
}


bool asset_table_erase(AssetTable *table, const asset_key_t key)
{
    void* value = asset_table_steal(table, key);
    if (value != NULL && table->freeFunc != NULL)
        table->freeFunc(value);
    return value != NULL;
}
