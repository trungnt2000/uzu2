#include "ecs_priv.h"
#include "toolbox/memory.h"

/**
 * i1 means sparse index 1
 * j1 means dense index 1
 */

#define PAGE_CNT ECS_PAGE_COUNT

#define PAGE_SIZ ECS_PAGE_SIZE

#define TOMBSTONE ECS_NULL_IDX

#define PAGE(idx) ((idx) / PAGE_SIZ)
#define OFFSET(idx) ((idx) & (PAGE_SIZ - 1u))

#define ALIGNMENT_EXTRA_SPACE 64u

static inline void
dataswp(u8* data, register size_t ts, u32 j1, u32 j2)
{
    register u8* mem1 = data + ts * j1;
    register u8* mem2 = data + ts * j2;

    do
    {
        u8 tmp  = *mem1;
        *mem1++ = *mem2;
        *mem2++ = tmp;
    } while (--ts > 0);
}

static inline void
datacpy(u8* data, size_t ts, u32 dst_index, u32 src_index)
{
    u8* src_mem = data + ts * src_index;
    u8* dst_mem = data + ts * dst_index;
    SDL_memcpy(dst_mem, src_mem, ts);
}

static inline ecs_size_t*
assure_page(ecs_size_t** sparse, u32 page)
{
    ASSERT(page < PAGE_CNT);
    if (sparse[page] == NULL)
    {
        sparse[page]    = SDL_malloc(sizeof(ecs_size_t) * PAGE_SIZ);
        ecs_size_t* mem = sparse[page];
        for (u32 i = 0; i < PAGE_SIZ; ++i)
            mem[i] = TOMBSTONE;
    }
    return sparse[page];
}

static void*
assure_alignment(void* buffer, size_t alignment, size_t data_size)
{
    size_t buffer_size = data_size + ALIGNMENT_EXTRA_SPACE;
    if (align(alignment, data_size, &buffer, &buffer_size) == NULL)
    {
        ASSERT_MSG(false, "extra space is too small for data to be aligned");
    }
    return buffer;
}

static inline void*
ecs_pool_component_offset(struct ecs_Pool* p, ecs_size_t idx)
{
    return (char*)p->data + p->traits.size * idx;
}

static inline void
ecs_pool_assure_size(struct ecs_Pool* p, ecs_size_t min_size)
{
    if (p->size < min_size)
    {
        const ecs_size_t new_size        = p->size * 2u;
        const size_t     new_data_size   = new_size * p->traits.size;
        const size_t     new_buffer_size = new_data_size + ALIGNMENT_EXTRA_SPACE;
        void*            new_buffer      = SDL_malloc(new_buffer_size);
        void*            new_data        = assure_alignment(new_buffer, p->traits.align, new_data_size);
        SDL_memcpy(new_data, p->data, p->size * p->traits.size);
        SDL_free(p->unaligned_buffer);
        p->data             = new_data;
        p->unaligned_buffer = new_buffer;
        p->size             = new_size;
        p->entities         = SDL_realloc(p->entities, new_size * sizeof(ecs_entity_t));
    }
}

struct ecs_Pool*
ecs_pool_create(struct ecs_Registry* registry, struct ecs_TypeTraits traits, ecs_size_t size)
{
    struct ecs_Pool* p = SDL_malloc(sizeof(struct ecs_Pool));

    /* initialize */
    p->entities = SDL_malloc(sizeof(ecs_entity_t) * size);
    p->registry = registry;

    const size_t data_size = traits.size * size;
    p->unaligned_buffer    = SDL_malloc(data_size + ALIGNMENT_EXTRA_SPACE);
    ASSERT_MSG(p->unaligned_buffer, "unable to allocate buffer");

    p->data = assure_alignment(p->unaligned_buffer, traits.align, data_size);

    p->size   = size;
    p->count  = 0;
    p->traits = traits;
    p->hook   = ECS_HOOK_EMPTY;

    for (int i = 0; i < ECS_SIG_CNT; ++i)
        ecs_signal_init(&p->signal[i]);

    SDL_memset(p->sparse, 0, sizeof(ecs_size_t*) * PAGE_CNT);
    for (u32 i = 0; i < p->size; ++i)
        p->entities[i] = ECS_NULL_ENT;
    return p;
}

void
ecs_pool_free(struct ecs_Pool* p)
{
    /* free page memory */
    for (u32 i = 0; i < PAGE_CNT; ++i)
        if (p->sparse[i] != NULL)
            SDL_free(p->sparse[i]);

    /* call destructor on all component */
    if (p->traits.fini != NULL)
    {
        u8* mem = p->data;
        for (u32 i = 0; i < p->count; ++i)
        {
            p->traits.fini(mem);
            mem = mem + p->traits.size;
        }
    }
    SDL_free(p->unaligned_buffer);
    SDL_free(p->entities);
    SDL_free(p);
}

void*
ecs_pool_add(struct ecs_Pool* p, ecs_entity_t ett)
{

    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);

    ecs_size_t* pagemem = assure_page(p->sparse, page);

    ASSERT(pagemem[offset] == TOMBSTONE);

    ecs_pool_assure_size(p, p->count + 1);

    ecs_size_t j    = p->count;
    pagemem[offset] = j;
    p->entities[j]  = ett;
    p->count++;

    if (p->hook.add != NULL)
        j = p->hook.add(p->hook.ctx, p, ett, j);

    void* component = ecs_pool_component_offset(p, j);

    ecs_signal_emit(&p->signal[ECS_SIG_ADD], p->registry, ett, component);
    return component;
}

void*
ecs_pool_addv(struct ecs_Pool* p, ecs_entity_t ett, const void* data)
{
    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);

    ecs_size_t* pagemem = assure_page(p->sparse, page);

    ASSERT(pagemem[offset] == TOMBSTONE);

    ecs_pool_assure_size(p, p->count + 1);

    ecs_size_t j    = p->count;
    pagemem[offset] = j;
    p->entities[j]  = ett;
    p->count++;

    SDL_memcpy(ecs_pool_component_offset(p, j), data, p->traits.size);

    if (p->hook.add != NULL)
        j = p->hook.add(p->hook.ctx, p, ett, j);

    void* component = ecs_pool_component_offset(p, j);

    ecs_signal_emit(&p->signal[ECS_SIG_ADD], p->registry, ett, component);
    return component;
}

void*
ecs_pool_assurev(struct ecs_Pool* p, ecs_entity_t ett, const void* data)
{
    void* component = ecs_pool_get(p, ett);
    return component ? component : ecs_pool_addv(p, ett, data);
}

void*
ecs_pool_assure(struct ecs_Pool* p, ecs_entity_t ett)
{
    void* component = ecs_pool_get(p, ett);
    return component ? component : ecs_pool_add(p, ett);
}

void*
ecs_pool_set(struct ecs_Pool* p, ecs_entity_t ett, const void* data)
{
    void* component = ecs_pool_get(p, ett);
    ASSERT_MSG(component != NULL, "This pool does not contains given entity!");
    return SDL_memcpy(component, data, p->traits.size);
}

void*
ecs_pool_get(struct ecs_Pool* p, ecs_entity_t ett)
{
    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);
    if (p->sparse[page] == NULL)
        return NULL;
    ecs_size_t j = p->sparse[page][offset];
    return j != TOMBSTONE ? ecs_pool_component_offset(p, j) : NULL;
}

void*
ecs_pool_cpy(struct ecs_Pool* p, ecs_entity_t ett, const void* other)
{
    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);

    ecs_size_t* pagemem = assure_page(p->sparse, page);
    ASSERT(pagemem[offset] == TOMBSTONE);

    ecs_pool_assure_size(p, p->count + 1);

    ecs_size_t j    = p->count;
    pagemem[offset] = j;
    p->entities[j]  = ett;
    void* mem       = ecs_pool_component_offset(p, j);
    if (p->traits.cpy != NULL)
        p->traits.cpy(mem, other);
    else
        SDL_memcpy(mem, other, p->traits.size);
    ecs_signal_emit(&p->signal[ECS_SIG_ADD], p->registry, ett, mem);

    if (p->hook.add != NULL)
        j = p->hook.add(p->hook.ctx, p, ett, j);
    p->count++;
    return ecs_pool_component_offset(p, j);
}

bool
ecs_pool_rmv(struct ecs_Pool* p, ecs_entity_t ett)
{
    u32        idx    = ECS_TO_INDEX(ett);
    u32        page   = PAGE(idx);
    u32        offset = OFFSET(idx);
    ecs_size_t i2, j1, j2;
    if (p->sparse[page] == NULL)
        return false;

    j1 = p->sparse[page][offset];
    if (j1 == TOMBSTONE)
        return false;

    j2 = p->count - 1;
    i2 = ECS_TO_INDEX(p->entities[j2]);

    void* mem = ecs_pool_component_offset(p, j1);
    ecs_signal_emit(&p->signal[ECS_SIG_RMV], p->registry, ett, mem);

    // call destructor if any
    if (p->traits.fini != NULL)
        p->traits.fini(mem);

    // invoke hook function
    if (p->hook.rmv != NULL)
        j1 = p->hook.rmv(p->hook.ctx, p, ett, j1);

    if (j1 != j2) /* already at the end of this pool */
    {
        p->entities[j1] = p->entities[j2];
        datacpy(p->data, p->traits.size, j1, j2);
        p->sparse[PAGE(i2)][OFFSET(i2)] = j1;
    }
    p->sparse[page][offset] = TOMBSTONE;
    p->count--;
    return true;
}

void
ecs_pool_swp(struct ecs_Pool* p, ecs_entity_t ett1, ecs_entity_t ett2)
{

    u32 i1 = ECS_TO_INDEX(ett1);
    u32 i2 = ECS_TO_INDEX(ett2);

    u32 page1   = PAGE(i1);
    u32 offset1 = OFFSET(i1);
    u32 page2   = PAGE(i2);
    u32 offset2 = OFFSET(i2);

    ecs_size_t j1 = p->sparse[page1][offset1];
    ecs_size_t j2 = p->sparse[page2][offset2];

    /* swap dense */
    p->entities[j1] = ett2;
    p->entities[j2] = ett1;
    dataswp(p->data, p->traits.size, j1, j2);

    /* swap sparse */
    p->sparse[page1][offset1] = j2;
    p->sparse[page2][offset2] = j1;
}

bool
ecs_pool_contains(struct ecs_Pool* p, ecs_entity_t ett)
{
    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);
    return (page < PAGE_CNT && p->sparse[page] != NULL && p->sparse[page][offset] != TOMBSTONE);
}

ecs_size_t
ecs_pool_count(struct ecs_Pool* pool)
{
    return pool->count;
}

void
ecs_pool_data(struct ecs_Pool*     pool,
              void**               data_ptr_return,
              const ecs_entity_t** ett_ptr_return,
              ecs_size_t*          count_return)
{
    if (data_ptr_return != NULL)
        *data_ptr_return = pool->data;

    if (ett_ptr_return != NULL)
        *ett_ptr_return = pool->entities;

    if (count_return != NULL)
        *count_return = pool->count;
}

void
ecs_pool_connect(struct ecs_Pool* p, int sig, ecs_Callback callback, void* ctx)
{
    ecs_signal_connect(&p->signal[sig], callback, ctx);
}

void
ecs_pool_disconnect(struct ecs_Pool* p, int sig, ecs_Callback callback, void* ctx)
{
    ecs_signal_disconnect(&p->signal[sig], callback, ctx);
}

const ecs_entity_t*
ecs_pool_ett_rbegin(struct ecs_Pool* pool)
{
    return &pool->entities[pool->count] - 1;
}

const ecs_entity_t*
ecs_pool_ett_rend(struct ecs_Pool* pool)
{
    return &pool->entities[-1];
}

void*
ecs_pool_data_rbegin(struct ecs_Pool* pool)
{
    return (u8*)pool->data + (pool->traits.size * pool->count) - pool->traits.size;
}

void*
ecs_pool_data_rend(struct ecs_Pool* pool)
{
    return (u8*)pool->data - pool->traits.size;
}

void
ecs_pool_set_hook(struct ecs_Pool* pool, struct ecs_Hook hook)
{
    pool->hook = hook;
}

ecs_size_t
ecs_pool_index(struct ecs_Pool* p, ecs_entity_t ett)
{
    u32 idx    = ECS_TO_INDEX(ett);
    u32 page   = PAGE(idx);
    u32 offset = OFFSET(idx);

    if (p->sparse[page] == NULL)
        return TOMBSTONE;
    return p->sparse[page][offset];
}

bool
ecs_pool_sortable(struct ecs_Pool* pool)
{
    return pool->hook.add == NULL && pool->hook.rmv == NULL;
}

void*
ecs_pool_data_begin(struct ecs_Pool* pool)
{
    return pool->data;
}

const ecs_entity_t*
ecs_pool_ett_begin(struct ecs_Pool* pool)
{
    return pool->entities;
}
