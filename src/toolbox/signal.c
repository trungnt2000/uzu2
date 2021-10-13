#include "toolbox/signal.h"
#include <stdlib.h>

#define SIGNAL_INITIAL_SIZE 8

struct Slot
{
    bool (*func)(void* ctx, const void* data);
    void* ctx;
    u32   flags;
    bool  deleted;
};

struct Signal
{
    u32          size;
    u32          count;
    struct Slot* slots;
    bool         is_emitting;
    bool         has_deleted_slot;
};

struct MsgBus
{
    u32           num_singals;
    struct Signal signals[];
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
    return signal_init(malloc(sizeof(struct Signal)));
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
    struct Slot* slots = signal->slots;

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
    signal->slots[signal->count++] = (struct Slot){ .func = func, .ctx = ctx, .flags = flags };
}

/* actualy remove deleted slots */
static void
signal_cleanup(Signal* signal)
{
    if (!signal->has_deleted_slot)
        return;

    u32          l     = 0;
    u32          r     = signal->count - 1;
    struct Slot* slots = signal->slots;
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
            struct Slot tmp = slots[l];
            slots[l]        = slots[r];
            slots[r]        = tmp;

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

    struct Slot* slots = signal->slots;
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
    struct Slot* iter   = &signal->slots[signal->count - 1];
    struct Slot* end    = &signal->slots[-1];
    bool         should_delete;
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
    signal_cleanup(signal);
}

MsgBus*
mbus_create(u32 num_signals)
{
    MsgBus* mbus      = malloc(sizeof(MsgBus) + sizeof(Signal) * num_signals);
    mbus->num_singals = num_signals;
    for (u32 i = 0; i < num_signals; ++i)
    {
        signal_init(&mbus->signals[i]);
    }
    return mbus;
}

void
mbus_free(MsgBus* mbus)
{
    if (mbus != NULL)
    {
        for (u32 i = 0; i < mbus->num_singals; ++i)
        {
            signal_fini(mbus->signals + i);
        }
        SDL_free(mbus);
    }
}

void
mbus_connect(MsgBus* mbus, int sig, bool (*func)(void*, const void*), void* ctx, u32 flags)
{
    signal_connect(&mbus->signals[sig], func, ctx, flags);
}

void
mbus_disconnect(MsgBus* mbus, int sig, bool (*func)(void*, const void*), void* ctx)
{
    signal_disconnect(&mbus->signals[sig], func, ctx);
}

void
mbus_emit(MsgBus* mbus, int sig, const void* data)
{
    signal_emit(&mbus->signals[sig], data);
}
