// ngotrung Fri 23 Jul 2021 07:41:56 PM +07
#ifndef ARRAY_H
#define ARRAY_H
#include "SDL.h"
#include <stdint.h>
typedef struct ArrayHeader
{
  uint32_t size;
  uint32_t count;
} ArrayHeader;

#define array_header(a) ((ArrayHeader*)(((char*)(a)) - sizeof(ArrayHeader)))
#define array_size(a) ((a) ? array_header(a)->size : 0)
#define array_count(a) ((a) ? array_header(a)->count : 0)

#define array_add(a, v)                                                        \
  (array_assure_size((a), array_count(a) + 1),                                 \
   (a)[array_header(a)->count++] = v)

#define array_assure_size(a, n) ((a) = _array_assure_size(a, sizeof(*(a)), n))

#define array_sink(a) _array_sink(a, sizeof(*(a)));

#define ARRAY_MIN_SIZE 16

static inline uint32_t
next_power_of_two(uint32_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

void*
_array_assure_size(void* a, SDL_UNUSED size_t elsize, uint32_t n)
{
  n = next_power_of_two(n);
  n = n > ARRAY_MIN_SIZE ? n : ARRAY_MIN_SIZE;
  if (array_size(a) < n)
  {
    a = SDL_realloc(a ? array_header(a) : NULL,
                    elsize * n + sizeof(ArrayHeader));

    a = (char*)a + sizeof(ArrayHeader);

    array_header(a)->size = n;
  }

  return a;
}

inline void*
_array_sink(void* a, size_t elsize)
{
  uint32_t currsiz = array_size(a);
  if (a && (currsiz >> 1) > array_count(a) && currsiz > ARRAY_MIN_SIZE)
  {
    uint32_t newsiz = currsiz >> 1;
    a = SDL_realloc(array_header(a), newsiz * elsize + sizeof(ArrayHeader));
    a = (char*)a + sizeof(ArrayHeader);
    array_header(a)->size = newsiz;
  }
  return a;
}

#include <stdlib.h>
int
main(int argc, char** argv)
{
  int* arr = NULL;
  array_add(arr, 1);
  array_add(arr, 2);
  array_add(arr, 3);
  for (int i = 0; i < array_size(arr); ++i)
  {
    printf("%d ", arr[i]);
  }
  printf("\n");
  return 0;
}

#endif // ARRAY_H
