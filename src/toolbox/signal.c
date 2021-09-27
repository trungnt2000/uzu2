#include "toolbox/signal.h"
#include <stdlib.h>

#define SIGNAL_INITIAL_SIZE 8

typedef struct Slot
{
  bool (*func)(void* ctx, const void* data);
  void* ctx;
  u32   flags;
  bool  deleted;
} Slot;

struct Signal
{
  u32   size;
  u32   count;
  Slot* slots;
  bool  is_emitting;
  bool  has_deleted_slot;
};

struct Emitter
{
  u32    num_singals;
  Signal signals[];
};

Signal*
signal_init(Signal* signal)
{
  signal->count            = 0;
  signal->size             = SIGNAL_INITIAL_SIZE;
  signal->slots            = SDL_calloc(SIGNAL_INITIAL_SIZE, sizeof(*signal->slots));
  signal->is_emitting      = false;
  signal->has_deleted_slot = false;
  return signal;
}

void
signal_fini(Signal* signal)
{
  SDL_free(signal->slots);
}

Signal*
signal_create()
{
  return signal_init(malloc(sizeof(Signal)));
}

void
signal_free(Signal* signal)
{
  if (signal != NULL)
  {
    signal_fini(signal);
    SDL_free(signal);
  }
}

static bool
signal_contains(Signal* signal, bool (*func)(void*, const void*), void* ctx)
{
  Slot* slots = signal->slots;

  for (u32 i = signal->count; i--;)
    if ((ctx == NULL || ctx == slots[i].ctx) && (func == NULL || func == slots[i].func))
      return true;
  return false;
}

void
signal_connect(Signal* signal, bool (*func)(void*, const void*), void* ctx, u32 flags)
{
  if (ctx == NULL && func == NULL)
    return;

  if (signal_contains(signal, func, ctx))
    return;

  if (signal->count == signal->size)
  {
    signal->size  = signal->size << 1;
    signal->slots = SDL_realloc(signal->slots, signal->size * sizeof(*signal->slots));
  }
  signal->slots[signal->count++] = (Slot){ .func = func, .ctx = ctx, .flags = flags };
}

/* actualy remove deleted slots */
static void
signal_do_delete(Signal* signal)
{
  if (!signal->has_deleted_slot)
    return;

  u32   l     = 0;
  u32   r     = signal->count - 1;
  Slot* slots = signal->slots;
  while (1)
  {

    // find deleted slot
    for (; l <= r; ++l)
      if (slots[l].deleted)
        break;

    // find alive slot
    for (; r > l; --r)
      if (!slots[r].deleted)
        break;

    if (slots[l].deleted && !slots[r].deleted)
    {
      Slot tmp = slots[l];
      slots[l] = slots[r];
      slots[r] = tmp;

      ++l;
      --r;
    }

    if (r <= l)
      break;
  }
  signal->count            = l;
  signal->has_deleted_slot = false;
}

bool
signal_disconnect(Signal* signal, bool (*func)(void*, const void*), void* ctx)
{
  if (ctx == NULL && func == NULL)
    return false;

  Slot* slots = signal->slots;
  for (u32 i = signal->count; i--;)
  {
    if ((ctx == NULL || ctx == slots[i].ctx) && (func == NULL || slots[i].func == func))
    {
      if (signal->is_emitting)
      {
        slots[i].deleted         = true;
        signal->has_deleted_slot = true;
      }
      else
      {
        slots[i] = slots[--signal->count];
      }
      return true;
    }
  }
  return false;
}

void
signal_emit(Signal* signal, const void* data)
{
  if (signal->count == 0)
    return;

  ASSERT(!signal->is_emitting); /* avoid recusively emiting */
  signal->is_emitting = true;
  Slot* iter          = &signal->slots[signal->count - 1];
  Slot* end           = &signal->slots[-1];
  bool  should_delete;
  for (; iter != end; --iter)
  {
    should_delete = iter->func(iter->ctx, data);
    should_delete |= iter->flags & SLOT_FLAG_ONESHOT;
    if (should_delete)
    {
      iter->deleted            = true;
      signal->has_deleted_slot = true;
    }
  }
  signal->is_emitting = false;
  signal_do_delete(signal);
}

Emitter*
emitter_create(u32 num_signals)
{
  Emitter* emitter     = malloc(sizeof(Emitter) + sizeof(Signal) * num_signals);
  emitter->num_singals = num_signals;
  for (u32 i = 0; i < num_signals; ++i)
  {
    signal_init(&emitter->signals[i]);
  }
  return emitter;
}

void
emitter_free(Emitter* emitter)
{
  if (emitter != NULL)
  {
    for (u32 i = 0; i < emitter->num_singals; ++i)
    {
      signal_fini(emitter->signals + i);
    }
    SDL_free(emitter);
  }
}

void
emitter_connect(Emitter* emitter, int sig, bool (*func)(void*, const void*), void* ctx, u32 flags)
{
  signal_connect(&emitter->signals[sig], func, ctx, flags);
}

void
emitter_disconnect(Emitter* emmiter, int sig, bool (*func)(void*, const void*), void* ctx)
{
  signal_disconnect(&emmiter->signals[sig], func, ctx);
}

void
emitter_emit(Emitter* emmiter, int sig, const void* data)
{
  signal_emit(&emmiter->signals[sig], data);
}
