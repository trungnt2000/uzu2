#include "ecs_priv.h"

#define INITIAL_SIZE 4

void
ecs_signal_init(struct ecs_Signal* signal)
{
    signal->count = 0;
    signal->size  = INITIAL_SIZE;
    signal->slots = SDL_malloc(sizeof(struct ecs_Slot) * INITIAL_SIZE);
}

void
ecs_signal_fini(struct ecs_Signal* signal)
{
    SDL_free(signal->slots);
}

static inline bool
check_slot_equal(struct ecs_Slot lhs, struct ecs_Slot rhs)
{
    return lhs.callback == rhs.callback && lhs.ctx == rhs.ctx;
}

static bool
slot_array_contains(struct ecs_Slot* slots, u32 n, struct ecs_Slot slot)
{
    for (u32 i = 0; i < n; ++i)
        if (check_slot_equal(slots[i], slot))
            return true;
    return false;
}

static void
ecs_signal_swap_pop(struct ecs_Signal* s, u32 i)
{
    s->slots[i] = s->slots[s->count - 1];
    s->count--;
}

static void
ecs_signal_assure_size(struct ecs_Signal* s, u32 n)
{
    if (n > s->size)
    {
        s->size <<= 1;
        s->slots = SDL_realloc(s->slots, s->size);
    }
}

void
ecs_signal_connect(struct ecs_Signal* signal, ecs_Callback callback, void* ctx)
{
    if (callback == NULL)
        return;

    ecs_signal_assure_size(signal, signal->count + 1);
    struct ecs_Slot slot = { callback, ctx };
    if (!slot_array_contains(signal->slots, signal->count, slot))
    {
        signal->slots[signal->count++] = slot;
    }
}

void
ecs_signal_disconnect(struct ecs_Signal* signal, ecs_Callback callback, void* ctx)
{
    struct ecs_Slot* slots = signal->slots;

    // remove all slot by both callback function & ctx
    if (callback != NULL && ctx != NULL)
    {
        struct ecs_Slot slot = { callback, ctx };
        for (u32 i = signal->count; --i;)
            if (check_slot_equal(slots[i], slot))
                ecs_signal_swap_pop(signal, i);
    }
    // remove all slot by given callback function
    else if (callback != NULL && ctx == NULL)
    {
        for (u32 i = signal->count; i--;)
            if (callback == slots[i].callback)
                ecs_signal_swap_pop(signal, i);
    }
    // remove all slot by given ctx
    else if (callback == NULL && ctx != NULL)
    {
        for (u32 i = signal->count; i--;)
            if (ctx == slots[i].ctx)
                ecs_signal_swap_pop(signal, i);
    }
}

void
ecs_signal_emit(struct ecs_Signal* signal, ecs_Registry* registry, ecs_entity_t entity, void* mem)
{
    const struct ecs_Slot* slots = signal->slots;

    for (u32 i = signal->count; i--;)
    {
        slots[i].callback(slots[i].ctx, registry, entity, mem);
    }
}
