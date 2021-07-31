#ifndef PTRARRAY_H
#define PTRARRAY_H
#include "common.h"
#include <stdlib.h>
typedef struct PtrArray
{
  pointer_t* storage;
  s32        count;
  s32        size;
  FreeFunc   freeFunc;
} PointerArray;

PointerArray* pointer_array_create(FreeFunc freeFunc);
PointerArray* pointer_array_create_sized(FreeFunc freeFunc, int size);
void          pointer_array_free(PointerArray* arr);

PointerArray* pointer_array_init(PointerArray* arr, FreeFunc freeFunc);

PointerArray*
pointer_array_init_sized(PointerArray* arr, FreeFunc freeFunc, s32 size);

void pointer_array_fini(PointerArray* arr);

pointer_t pointer_array_add(PointerArray* arr, pointer_t p);
bool      pointer_array_rmv(PointerArray* arr, pointer_t p);
void      pointer_array_rmv_idx(PointerArray* arr, s32 idx);
void      pointer_array_qrmv(PointerArray* arr, s32 idx);

int  pointer_array_find(PointerArray* arr, pointer_t p);
bool pointer_array_contains(PointerArray* arr, pointer_t p);
void pointer_array_reserve(PointerArray* arr, s32 n);
void pointer_array_sort(PointerArray* arr, CompareFunc comp);
void pointer_array_clear(PointerArray* arr);

#define ptr_array_storage(__T, __self) ((__T**)(__self)->storage)
#define ptr_array_at(__self, idx) ((__self)->storage[(idx)])

#endif // PTRARRAY_H
