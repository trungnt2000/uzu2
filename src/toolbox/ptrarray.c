#include "toolbox/ptrarray.h"
#include <stdlib.h>

PointerArray*
pointer_array_create(FreeFunc freeFunc)
{
  return pointer_array_create_sized(freeFunc, 16);
}

void
pointer_array_free(PointerArray* arr)
{
  if (arr)
  {
    pointer_array_fini(arr);
    free(arr);
  }
}

PointerArray*
pointer_array_init(PointerArray* self, FreeFunc freeFunc)
{
  return pointer_array_init_sized(self, freeFunc, 16);
}

PointerArray*
pointer_array_init_sized(PointerArray* self, FreeFunc freeFunc, s32 cap)
{
  self->size     = cap;
  self->count    = 0;
  self->freeFunc = freeFunc;
  self->storage  = SDL_malloc(sizeof(pointer_t) * self->size);
  return self;
}

void
pointer_array_fini(PointerArray* self)
{
  pointer_array_clear(self);
  free(self->storage);
}

pointer_t
pointer_array_add(PointerArray* self, pointer_t p)
{
  if (self->count == self->size)
    pointer_array_reserve(self, self->size * 2);
  return self->storage[self->count++] = p;
}

bool
pointer_array_rmv(PointerArray* self, pointer_t p)
{
  int idx = pointer_array_find(self, p);
  if (idx != -1)
  {
    pointer_array_rmv_idx(self, idx);
    return true;
  }
  return false;
}

void
pointer_array_rmv_idx(PointerArray* self, s32 idx)
{
  ASSERT(idx >= 0 && idx < self->count && "out of index");
  int        cnt     = self->count;
  pointer_t* storage = self->storage;
  if (self->freeFunc)
    self->freeFunc(storage[idx]);

  for (int i = idx; i < cnt - 1; ++i)
    storage[i] = storage[i + 1];

  --self->count;
}

void
pointer_array_qrmv(PointerArray* self, s32 idx)
{
  ASSERT(idx >= 0 && idx < self->count && "out of index");
  if (self->freeFunc != NULL)
    self->freeFunc(self->storage[idx]);

  self->storage[idx] = self->storage[self->count - 1];
  self->count--;
}

int
pointer_array_find(PointerArray* self, pointer_t p)
{
  int        cnt = self->count;
  pointer_t* a   = self->storage;
  for (int i = 0; i < cnt; ++i)
    if (a[i] == p)
      return i;
  return -1;
}

bool
pointer_array_contains(PointerArray* self, pointer_t p)
{
  return pointer_array_find(self, p) != -1;
}

void
pointer_array_reserve(PointerArray* self, s32 n)
{
  if (n <= self->size)
    return;
  self->size    = n;
  self->storage = SDL_realloc(self->storage, self->size * sizeof(pointer_t));
}

void
pointer_array_sort(PointerArray* self, CompareFunc compareFunc)
{
  SDL_qsort(self->storage, self->count, sizeof(pointer_t), compareFunc);
}

void
pointer_array_clear(PointerArray* self)
{
  if (self->freeFunc != NULL)
    for (s32 i = 0; i < self->count; ++i)
      self->freeFunc(self->storage[i]);

  self->count = 0;
}

PointerArray*
pointer_array_create_sized(FreeFunc freeFunc, int size)
{
  PointerArray* self = SDL_malloc(sizeof(PointerArray));
  return pointer_array_init_sized(self, freeFunc, size);
}
