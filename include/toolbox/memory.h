#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"
#include <stddef.h>

INLINE void*
align(size_t align, size_t size, void** ptr, size_t* space)
{
  if (*space < size)
    return NULL;
  const uintptr_t intptr = (uintptr_t)(*ptr);
  const uintptr_t aligned = (intptr - 1u + align) & (1u + ~align);
  const ptrdiff_t diff = aligned - intptr;
  if (diff > (ptrdiff_t)(*space - size)) {
    return NULL;
  }
  else {
    *space -= diff;
    return *ptr = (void*)(aligned);
  }
}


#endif // !MEMORY_H