#include <SDL2/SDL.h>
#include "toolbox/common.h"

typedef struct HashTable HashTable;
typedef void (*HashTableCallback)(void* arg, void* key, void* value);

u32 strhash(const void* arg);
u32 inthash(const void* arg);
u32 longhash(const void* arg);
u32 ptrhash(const void* arg);

HashTable*
     hash_table_create(HashFunc hashFunc, EqualFunc equalFunc, FreeFunc freeFunc);
void hash_table_free(HashTable* ht);
void hash_table_free_null(HashTable** ht);

void  hash_table_insert(HashTable* ht, void* key, void* value);
BOOL  hash_table_remove(HashTable* ht, void* key);
void* hash_table_steal(HashTable* ht, void* key);
void* hash_table_lookup(HashTable* ht, void* key);
u32   hash_table_count(HashTable* ht);
u32   hash_table_size(HashTable* ht);
void  hash_table_rehash(HashTable* ht);
void  hash_table_foreach(HashTable* ht, HashTableCallback cb, void* arg);
