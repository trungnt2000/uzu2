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

/* return if new slot is used */
static bool
asset_table_set_impl(asset_key_t*      keys,
                     u32*              hashes,
                     void**            values,
                     u32               hash_value,
                     const asset_key_t key,
                     void*             value,
                     u32               size)
{
    const u32 mask             = size - 1;
    u32       index            = hash_value & mask;
    bool      has_deleted_slot = false;
    u32       first_deleted_slot;
    u32       x = 0;

    // probing until find empty slot
    // or find slot which has same key
    while (!IS_EMPTY_SLOT(hashes[index]))
    {
        // keep going to prevent duplicate key
        if (IS_DELETED_SLOT(hashes[index]) && !has_deleted_slot)
        {
            has_deleted_slot   = true;
            first_deleted_slot = index;
        }
        // key already exist in this table replace with new value
        else if (hashes[index] == hash_value && asset_key_equals(keys[index], key))
        {
            asset_key_copy(keys[index], key);
            values[index] = value;
            hashes[index] = hash_value;
            return false;
        }
        x++;
        index = (index + x) & mask;
    }
    if (has_deleted_slot)
        index = first_deleted_slot;

    asset_key_copy(keys[index], key);
    values[index] = value;
    hashes[index] = hash_value;
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

    tbl->tombstone_threshold = size * 3 / 16;
    tbl->rehash_at           = size * 12 / 16;
    tbl->size                = size;
    tbl->tombstone_count     = 0;
}

void
asset_table_init(AssetTable* tbl, FreeFunc free_func)
{
    tbl->size                = 16;
    tbl->keys                = SDL_calloc(tbl->size, sizeof *tbl->keys);
    tbl->hashes              = SDL_calloc(tbl->size, sizeof *tbl->hashes);
    tbl->values              = SDL_calloc(tbl->size, sizeof *tbl->values);
    tbl->tombstone_count     = 0;
    tbl->tombstone_threshold = tbl->size * 3 / 16;
    tbl->count               = 0;
    tbl->rehash_at           = tbl->size * 12 / 16;
    tbl->free_func           = free_func;
}

void
asset_table_destroy(AssetTable* tbl)
{
    if (tbl == NULL)
        return;
    if (tbl->free_func && tbl->hashes)
    {
        for (u32 i = 0; i < tbl->size; ++i)
        {
            if (IS_USED_SLOT(tbl->hashes[i]))
            {
                tbl->free_func(tbl->values[i]);
            }
        }
    }
    SDL_free(tbl->keys);
    SDL_free(tbl->values);
    SDL_free(tbl->hashes);
    tbl->free_func = NULL;
    tbl->hashes    = NULL;
    tbl->keys      = NULL;
    tbl->values    = NULL;
}

void
asset_table_insert(AssetTable* tbl, const asset_key_t key, void* value)
{
    const u32 hash_value = asset_key_hash(key);
    ASSERT(hash_value != TOMBSTONE && hash_value != EMPTY);
    if (asset_table_set_impl(tbl->keys, tbl->hashes, tbl->values, hash_value, key, value, tbl->size))
    {
        tbl->count++;
        if (tbl->count > tbl->rehash_at)
        {
            asset_table_rehash(tbl, tbl->size * 2);
        }
    }
}

void*
asset_table_lookup(const AssetTable* tbl, const asset_key_t key)
{
    const u32          mask       = tbl->size - 1;
    const u32*         hashes     = tbl->hashes;
    const asset_key_t* keys       = tbl->keys;
    const u32          hash_value = asset_key_hash(key);
    u32                idx        = hash_value & mask;
    u32                x          = 0;

    ASSERT(hash_value != TOMBSTONE && hash_value != EMPTY);
    while (!IS_EMPTY_SLOT(hashes[idx]))
    {
        if (hashes[idx] == hash_value && asset_key_equals(keys[idx], key))
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
    const u32          hash_value = asset_key_hash(key);
    const u32          size       = tbl->size;
    u32                x          = 0;
    u32*               hashes     = tbl->hashes;
    const asset_key_t* keys       = tbl->keys;
    void*              retVal;
    const u32          mask = size - 1;
    u32                idx  = hash_value & mask;

    while (!IS_EMPTY_SLOT(hashes[idx]))
    {
        if (IS_USED_SLOT(hashes[idx]) && hashes[idx] == hash_value && asset_key_equals(keys[idx], key))
        {
            hashes[idx] = TOMBSTONE;
            tbl->tombstone_count++;
            tbl->count--;
            retVal = tbl->values[idx];
            if (tbl->tombstone_count > tbl->tombstone_threshold)
                asset_table_rehash(tbl, tbl->size);
            return retVal;
        }
        x++;
        idx = (idx + x) & mask;
    }
    return NULL;
}

bool
asset_table_erase(AssetTable* table, const asset_key_t key)
{
    void* value = asset_table_steal(table, key);
    if (value != NULL && table->free_func != NULL)
        table->free_func(value);
    return value != NULL;
}
