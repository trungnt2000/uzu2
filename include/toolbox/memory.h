#ifndef MEMORY_H
#define MEMORY_H
#include "common.h"
#include <stddef.h>

INLINE void*
align(size_t align, size_t size, void** ptr, size_t* space)
{
    if (*space < size)
        return NULL;
    const uintptr_t intptr  = (uintptr_t)(*ptr);
    const uintptr_t aligned = (intptr - 1u + align) & (1u + ~align);
    const ptrdiff_t diff    = aligned - intptr;
    if (diff > (ptrdiff_t)(*space - size))
    {
        return NULL;
    }
    else
    {
        *space -= diff;
        return *ptr = (void*)(aligned);
    }
}

INLINE void*
SDL_aligned_alloc(size_t size, size_t align)
{

    uintptr_t real_adr = (uintptr_t)SDL_malloc(size + align + sizeof(void*));

    if ((void*)real_adr == NULL)
        return NULL;

    uintptr_t aligned_adr = ((real_adr + sizeof(void*)) - 1u + align) & (1u + ~align);

    ((void**)aligned_adr)[-1] = (void*)real_adr;

    return (void*)aligned_adr;
}

INLINE void
SDL_aligned_free(void* mem)
{
    if (mem == NULL)
        return;
    uintptr_t aligned_adr = (uintptr_t)mem;
    void*     real_adr    = ((void**)aligned_adr)[-1];
    SDL_free(real_adr);
}

#endif // !MEMORY_H
