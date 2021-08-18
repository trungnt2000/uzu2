#include "assert_manager.h"

#define TOMBSTONE 1
#define LOAD_FACTOR 0.75
#define EMPTY 0

#define IS_DELETED_SLOT(x) ((x) == TOMBSTONE)
#define IS_EMPTY_SLOT(x) ((x) == EMPTY)
#define IS_USED_SLOT(x) ((x) != EMPTY && (x) != TOMBSTONE)

static u32
jenkins_one_at_a_time_hash(const char* key)
{
  u32 hash = 13;
  while (*key)
  {
    hash += *key++;
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
  int maxlen = (int)sizeof(asset_key_t) - 1;
  for (int i = 0; i < maxlen && lhs[i] && rhs[i]; ++i)
  {
    if (lhs[i] != rhs[i])
      return false;
  }
  return true;
}

static void
asset_key_copy(asset_key_t dst, const asset_key_t src)
{
  SDL_strlcpy(dst, src, sizeof(asset_key_t) - 1);
}

static bool
asset_table_insert_impl(asset_key_t*      keys,
                        u32*              hashes,
                        void**            values,
                        u32               hashValue,
                        const asset_key_t key,
                        void*             value,
                        u32               size)
{
  u32  index          = hashValue & (size - 1);
  bool hasDeletedSlot = false;
  u32  firstDeletedSlot;
  u32  x = 0;

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
    index = (index + x) & (size - 1);
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
      asset_table_insert_impl(keys,
                              hashes,
                              tbl->values,
                              tbl->hashes[i],
                              tbl->keys[i],
                              tbl->values[i],
                              size);
    }
  }
  SDL_free(tbl->keys);
  SDL_free(tbl->hashes);
  SDL_free(tbl->values);

  tbl->hashes                = hashes;
  tbl->keys                  = keys;
  tbl->values                = values;
  tbl->tombstoneCnt          = 0;
  tbl->tombstoneCntThreshold = size * 3 / 16;
  tbl->usedCntThreshold      = size * 12 / 16;
  tbl->size                  = size;
}

void
asset_table_init(AssetTable* tbl)
{
  tbl->size                  = 16;
  tbl->keys                  = SDL_calloc(tbl->size, sizeof *tbl->keys);
  tbl->hashes                = SDL_calloc(tbl->size, sizeof *tbl->hashes);
  tbl->values                = SDL_calloc(tbl->size, sizeof *tbl->values);
  tbl->tombstoneCnt          = 0;
  tbl->tombstoneCntThreshold = tbl->size * 3 / 16;
  tbl->usedCnt               = 0;
  tbl->usedCntThreshold      = tbl->size * 12 / 16;
}

void
asset_table_destroy(AssetTable* tbl)
{
  SDL_free(tbl->keys);
  SDL_free(tbl->values);
  SDL_free(tbl->hashes);
}

void
asset_table_insert(AssetTable* tbl, const asset_key_t key, void* value)
{
  const u32 hashValue = asset_key_hash(key);
  ASSERT(hashValue != TOMBSTONE && hashValue != EMPTY);
  bool newlyInserted = asset_table_insert_impl(tbl->keys,
                                               tbl->hashes,
                                               tbl->values,
                                               hashValue,
                                               key,
                                               value,
                                               tbl->size);
  if (newlyInserted)
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
  const u32          size      = tbl->size;
  const u32*         hashes    = tbl->hashes;
  const asset_key_t* keys      = tbl->keys;
  const u32          hashValue = asset_key_hash(key);
  u32                idx       = hashValue & (size - 1);
  u32                x         = 0;

  ASSERT(hashValue != TOMBSTONE && hashValue != EMPTY);
  while (!IS_EMPTY_SLOT(hashes[idx]))
  {
    if (hashes[idx] == hashValue && asset_key_equals(keys[idx], key))
    {
      return tbl->values[idx];
    }

    x++;
    idx = (idx + x) & (size - 1);
  }
  return NULL;
}

void*
asset_table_erase(AssetTable* tbl, const asset_key_t key)
{
  const u32          hashValue = asset_key_hash(key);
  const u32          size      = tbl->size;
  u32                idx       = hashValue & (size - 1);
  u32                x         = 0;
  u32*               hashes    = tbl->hashes;
  const asset_key_t* keys      = tbl->keys;
  void*              retVal;

  while (!IS_EMPTY_SLOT(hashes[idx]))
  {
    if (IS_USED_SLOT(hashes[idx]) && hashes[idx] == hashValue &&
        asset_key_equals(keys[idx], key))
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
    idx = (idx + x) & (size - 1);
  }
  return NULL;
}
