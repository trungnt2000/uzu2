#include "ecs_priv.h"

#define INITIAL_SIZE 4

void
ecs_signal_init(ecs_Signal* signal)
{
  signal->count = 0;
  signal->size  = INITIAL_SIZE;
  signal->slots = SDL_malloc(sizeof(ecs_Slot) * INITIAL_SIZE);
}

void
ecs_signal_fini(ecs_Signal* signal)
{
  SDL_free(signal->slots);
}

static inline bool
is_equal_slot(ecs_Slot lhs, ecs_Slot rhs)
{
  return lhs.callback == rhs.callback && lhs.ctx == rhs.ctx;
}

static bool
contains(ecs_Slot* slots, u32 n, ecs_Slot slot)
{
  for (u32 i = 0; i < n; ++i)
    if (is_equal_slot(slots[i], slot))
      return true;
  return false;
}

static void
swap_pop(ecs_Signal* s, int i)
{
  s->slots[i] = s->slots[s->count - 1];
  s->count--;
}

static void
maybe_grow(ecs_Signal* s, int n)
{
  u32 minSize = s->count + n;
  if (minSize > s->size)
  {
    s->size <<= 1;
    s->slots = SDL_realloc(s->slots, s->size);
  }
}

void
ecs_signal_connect(ecs_Signal* signal, ecs_Callback callback, void* ctx)
{
  if (callback == NULL)
    return;

  maybe_grow(signal, 1);
  ecs_Slot slot = { callback, ctx };
  if (!contains(signal->slots, signal->count, slot))
  {
    signal->slots[signal->count++] = slot;
  }
}

void
ecs_signal_disconnect(ecs_Signal* signal, ecs_Callback callback, void* ctx)
{
  u32       n     = signal->size;
  ecs_Slot* slots = signal->slots;

  // remove all slot by both callback function & ctx
  if (callback != NULL && ctx != NULL)
  {
    ecs_Slot slot = { callback, ctx };
    for (int i = n - 1; i >= 0; ++i)
      if (is_equal_slot(slots[i], slot))
        swap_pop(signal, i);
  }
  // remove all slot by given callback function
  else if (callback != NULL && ctx == NULL)
  {
    for (int i = n - 1; i >= 0; ++i)
      if (callback == slots[i].callback)
        swap_pop(signal, i);
  }
  // remove all slot by given ctx
  else if (callback == NULL && ctx != NULL)
  {
    for (int i = n - 1; i >= 0; ++i)
      if (ctx == slots[i].ctx)
        swap_pop(signal, i);
  }
}

void
ecs_signal_emit(ecs_Signal* signal, ecs_entity_t entity, void* mem)
{
  int       i     = signal->count - 1;
  ecs_Slot* slots = signal->slots;

  for (; i >= 0; --i)
    slots[i].callback(slots[i].ctx, entity, mem);
}
