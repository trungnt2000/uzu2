#include "toolbox/hash_table.h"

static const u32 _primeNumberTable[] = {
  1,         2,         3,          7,         13,       31,       61,
  127,       251,       509,        1021,      2039,     4093,     8191,
  16381,     32749,     65521,      131071,    262139,   524287,   1048573,
  2097143,   4194301,   8388593,    16777213,  33554393, 67108859, 134217689,
  268435399, 536870909, 1073741789, 2147483647
};

typedef struct HashTableEntry
{
  void*                  key;
  void*                  value;
  u32                    hashCode;
  struct HashTableEntry* next;
} HashTableEntry;

typedef struct HashTable
{
  HashFunc         hashFunc;
  EqualFunc        equalFunc;
  FreeFunc         freeFunc;
  u32              count;
  u32              size;
  u32              shift;
  double           maxLoadFactor;
  HashTableEntry** entries;
} HashTable;

static HashTableEntry*
hash_table_entry_create(void* key, void* value, u32 hashCode)
{
  HashTableEntry* e = SDL_malloc(sizeof(HashTableEntry));
  e->key            = key;
  e->value          = value;
  e->next           = NULL;
  e->hashCode       = hashCode;
  return e;
}

static HashTableEntry*
hash_table_entry_copy(const HashTableEntry* other)
{
  return hash_table_entry_create(other->key, other->value, other->hashCode);
}

#define ENTRY_LIST_FOREACH(head, it, ...)                                      \
  do                                                                           \
  {                                                                            \
    HashTableEntry* it = head;                                                 \
    while (it)                                                                 \
    {                                                                          \
      __VA_ARGS__                                                              \
      it = it->next;                                                           \
    }                                                                          \
  } while (0)

static void
entry_list_insert(HashTableEntry** headRef, HashTableEntry* entry)
{
  if (*headRef == NULL)
  {
    *headRef = entry;
  }
  else
  {
    entry->next = *headRef;
    *headRef    = entry;
  }
}

static HashTableEntry*
entry_list_find(HashTableEntry*  head,
                void*            key,
                u32              hashCode,
                EqualFunc        equalFunc,
                HashTableEntry** outPrevEntry)
{
  HashTableEntry* prevEntry = NULL;
  ENTRY_LIST_FOREACH(head, it, {
    if (it->hashCode == hashCode && equalFunc(it->key, key))
    {
      if (outPrevEntry)
        *outPrevEntry = prevEntry;
      return it;
    }
    prevEntry = it;
  });
  return NULL;
}

static void
entry_list_remove(HashTableEntry** headRef,
                  HashTableEntry*  entry,
                  HashTableEntry*  prevEntry)
{
  if (entry == *headRef)
    *headRef = entry->next;
  else
    prevEntry->next = entry->next;
}

INLINE double
hash_table_loadfactor(HashTable* self)
{
  return (double)self->count / (double)self->size;
}

HashTable*
hash_table_create(HashFunc hashFunc, EqualFunc equalFunc, FreeFunc freeFunc)
{
  HashTable* self     = SDL_malloc(sizeof(HashTable));
  self->shift         = 4;
  self->size          = _primeNumberTable[self->shift];
  self->count         = 0;
  self->freeFunc      = freeFunc;
  self->equalFunc     = equalFunc;
  self->hashFunc      = hashFunc;
  self->maxLoadFactor = 0.75;
  self->entries       = SDL_calloc(self->size, sizeof(HashTableEntry*));

  return self;
}

void
hash_table_free(HashTable* ht)
{
  if (ht)
  {
    HashTableEntry* it;
    HashTableEntry* next;
    for (u32 i = 0; i < ht->size; ++i)
    {
      it = ht->entries[i];
      while (it)
      {
        if (ht->freeFunc)
          ht->freeFunc(it->value);
        next = it->next;
        SDL_free(it);
        it = next;
      }
    }
    SDL_free(ht);
  }
}

void
hash_table_free_null(HashTable** table)
{
  hash_table_free(*table);
  *table = NULL;
}

void
hash_table_insert(HashTable* self, void* key, void* value)
{
  u32             hashCode, index;
  HashTableEntry* entry;

  hashCode = self->hashFunc(key);
  index    = hashCode % self->size;
  if ((entry = entry_list_find(self->entries[index],
                               key,
                               hashCode,
                               self->equalFunc,
                               NULL)) != NULL)
  {
    entry->value = value;
  }
  else
  {
    entry_list_insert(&self->entries[index],
                      hash_table_entry_create(key, value, hashCode));
    ++self->count;
  }

  if (hash_table_loadfactor(self) >= self->maxLoadFactor)
    hash_table_rehash(self);
}

void
hash_table_rehash(HashTable* self)
{
  u32              newSize  = _primeNumberTable[++self->shift];
  HashTableEntry** newTable = SDL_calloc(newSize, sizeof(HashTableEntry*));
  HashTableEntry*  it;
  u32              index;
  for (u32 i = 0; i < self->size; ++i)
  {
    it = self->entries[i];
    while (it)
    {
      index = it->hashCode % newSize;
      entry_list_insert(&newTable[index], hash_table_entry_copy(it));
      it = it->next;
    }
  }

  HashTableEntry* next;
  for (u32 i = 0; i < self->size; ++i)
  {
    it = self->entries[i];
    while (it)
    {
      next = it->next;
      SDL_free(it);
      it = next;
    }
  }

  self->entries = newTable;
  self->size    = newSize;
}

void*
hash_table_lookup(HashTable* self, void* key)
{
  HashTableEntry* entry;
  u32             hashCode, index;

  hashCode = self->hashFunc(key);
  index    = hashCode % self->size;
  if ((entry = entry_list_find(self->entries[index],
                               key,
                               hashCode,
                               self->equalFunc,
                               NULL)) != NULL)
    return entry->value;
  return NULL;
}

BOOL
hash_table_remove(HashTable* self, void* key)
{
  HashTableEntry* entry;
  HashTableEntry* prevEntry;
  u32             hashCode, index;

  hashCode = self->hashFunc(key);
  index    = hashCode % self->size;
  if ((entry = entry_list_find(self->entries[index],
                               key,
                               hashCode,
                               self->equalFunc,
                               &prevEntry)) != NULL)
  {
    entry_list_remove(&self->entries[index], entry, prevEntry);
    if (self->freeFunc)
      self->freeFunc(entry->value);
    SDL_free(entry);
    --self->count;
    return UZU_TRUE;
  }
  return UZU_FALSE;
}

void*
hash_table_steal(HashTable* self, void* key)
{
  HashTableEntry* entry;
  HashTableEntry* prevEntry;
  u32             hashCode, index;

  hashCode = self->hashFunc(key);
  index    = hashCode % self->size;
  if ((entry = entry_list_find(self->entries[index],
                               key,
                               hashCode,
                               self->equalFunc,
                               &prevEntry)) != NULL)
  {
    entry_list_remove(&self->entries[index], entry, prevEntry);
    SDL_free(entry);
    --self->count;
    return entry->value;
  }
  return NULL;
}

void
hash_table_foreach(HashTable* self, HashTableCallback callback, void* arg)
{
  HashTableEntry* it;
  for (u32 i = 0; i < self->size; ++i)
  {
    it = self->entries[i];
    while (it)
    {
      callback(arg, it->key, it->value);
      it = it->next;
    }
  }
}

u32
hash_table_count(HashTable* self)
{
  return self->count;
}

u32
hash_table_size(HashTable* self)
{
  return self->size;
}
