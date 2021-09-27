#include "ecs.h"
#include "ecs_priv.h"

#define INITIAL_SIZE 16

static void
clear_callback(void* ctx, struct ecs_Registry* registry, ecs_entity_t ett, SDL_UNUSED void* unused)
{
    ecs_destroy(registry, ett);
}

struct ecs_Registry*
ecs_registry_create(const struct ecs_TypeTraits* type_traits, ecs_size_t cnt)
{
    struct ecs_Registry* reg = SDL_malloc(sizeof(struct ecs_Registry));
    reg->type_count          = cnt;
    reg->type_traits         = type_traits;
    reg->pools               = SDL_calloc(cnt, sizeof(void*));
    reg->group_count         = 0;
    reg->groups              = NULL;
    for (int i = 0; i < cnt; ++i)
    {
        reg->pools[i] = ecs_pool_create(reg, type_traits[i], INITIAL_SIZE);
    }
    reg->count         = 0;
    reg->size          = INITIAL_SIZE;
    reg->destroy_index = 0;
    reg->entities      = SDL_malloc(INITIAL_SIZE * sizeof(ecs_entity_t));

    for (u32 i = 0; i < INITIAL_SIZE - 1; ++i)
        reg->entities[i] = ECS_ENT(i + 1, 0);

    reg->entities[INITIAL_SIZE - 1] = ECS_ENT(ECS_NULL_IDX, 0);
    return reg;
}

void
ecs_registry_free(struct ecs_Registry* reg)
{
    if (reg != NULL)
    {
        ecs_each(reg, clear_callback, NULL);
        for (int i = 0; i < reg->group_count; ++i)
        {
            struct ecs_Group* group = reg->groups[i];
            struct ecs_Group* child = NULL;
            while (group)
            {
                child = group->child;
                ecs_group_free(group);
                group = child;
            }
        }
        for (int i = 0; i < reg->type_count; ++i)
            ecs_pool_free(reg->pools[i]);
        SDL_free(reg->pools);
        SDL_free(reg->entities);
        SDL_free(reg);
    }
}

ecs_entity_t
ecs_create(struct ecs_Registry* reg)
{
    ecs_size_t ver;
    ecs_size_t idx;

    if (reg->destroy_index == ECS_NULL_IDX)
    {
        reg->size *= 2;
        reg->entities = SDL_realloc(reg->entities, reg->size * sizeof(ecs_entity_t));
        for (u32 i = reg->count; i < reg->size - 1; ++i)
        {
            reg->entities[i] = ECS_ENT(i + 1, 0);
        }
        reg->entities[reg->size - 1] = ECS_ENT(ECS_NULL_IDX, 0);
        reg->destroy_index           = reg->count;
    }

    idx                = reg->destroy_index;
    ver                = ECS_TO_VERSION(reg->entities[idx]);
    reg->destroy_index = ECS_TO_INDEX(reg->entities[idx]);
    reg->entities[idx] = ECS_TO_ENTITY(idx, ver);
    reg->count++;

    return ECS_TO_ENTITY(idx, ver);
}

INLINE void
recycle(struct ecs_Registry* reg, ecs_entity_t ett)
{
    ecs_size_t ent_ver;
    ecs_size_t ent_idx;

    ent_ver = ECS_TO_VERSION(ett);
    ent_idx = ECS_TO_INDEX(ett);

    reg->entities[ent_idx] = ECS_TO_ENTITY(reg->destroy_index, ent_ver + 1);
    reg->destroy_index     = ent_idx;
    reg->count--;
}

void
ecs_destroy(struct ecs_Registry* reg, ecs_entity_t ett)
{
    ecs_rmv_all(reg, ett);
    recycle(reg, ett);
}

void*
_ecs_add(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    return ecs_pool_add(reg->pools[type_id], ett, NULL);
}

void*
_ecs_add_ex(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id, const void* data)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    return ecs_pool_add(reg->pools[type_id], ett, data);
}

void*
_ecs_add_or_set(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id, const void* data)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    return ecs_pool_add_or_set(reg->pools[type_id], ett, data);
}

void
ecs_rmv(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    ecs_pool_rmv(reg->pools[type_id], ett);
}

void
ecs_rmv_all(struct ecs_Registry* reg, ecs_entity_t ett)
{
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    struct ecs_Pool** pools;

    pools = reg->pools;
    for (int i = 0; i < reg->type_count; ++i)
    {
        ecs_pool_rmv(pools[i], ett);
    }
}

void*
_ecs_get(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    return ecs_pool_get(reg->pools[type_id], ett);
}

void*
_ecs_get_or_add(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    void* comp = ecs_pool_get(reg->pools[type_id], ett);
    return comp ? comp : ecs_pool_add(reg->pools[type_id], ett, NULL);
}
void
ecs_each(struct ecs_Registry* reg, ecs_Callback callback, void* ctx)
{
    ecs_entity_t* entities;

    entities = reg->entities;
    if (reg->destroy_index == ECS_NULL_IDX)
    {
        for (ecs_size_t i = reg->size; i--;)
            callback(ctx, reg, entities[i], NULL);
    }
    else
    {
        for (ecs_size_t i = reg->size; i--;)
            if (ECS_TO_INDEX(entities[i]) == (ecs_size_t)i)
                callback(ctx, reg, entities[i], NULL);
    }
}

void
_ecs_raw(struct ecs_Registry* reg,
         ecs_size_t           type_id,
         const ecs_entity_t** etts,
         void*                comps[],
         ecs_size_t*          cnt)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ecs_pool_data(reg->pools[type_id], comps, etts, cnt);
}

bool
ecs_has(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");
    return ecs_pool_contains(reg->pools[type_id], ett);
}

void
ecs_clear(struct ecs_Registry* reg)
{
    ecs_each(reg, clear_callback, reg);
}

void*
_ecs_set(struct ecs_Registry* reg, ecs_entity_t ett, ecs_size_t type_id, const void* data)
{
    ASSERT_MSG(type_id < reg->type_count, "invalid type_id");
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");

    return ecs_pool_set(reg->pools[type_id], ett, data);
}

void
_ecs_getn(struct ecs_Registry* reg, ecs_entity_t ett, const ecs_size_t* types, ecs_size_t cnt, void* arr[])
{
    ASSERT_MSG(ecs_is_valid(reg, ett), "invalid entity");

    u32               type_count = reg->type_count;
    struct ecs_Pool** pools      = reg->pools;
    for (int i = 0; i < cnt; ++i)
    {
        ASSERT(types[i] < type_count);
        if ((arr[i] = ecs_pool_get(pools[types[i]], ett)) == NULL)
            break;
    }
}

ecs_entity_t
ecs_cpy(struct ecs_Registry* dst_reg, struct ecs_Registry* src_reg, ecs_entity_t src_ett)
{
    ecs_entity_t cpy_ett;
    void*        src_mem;
    ASSERT(dst_reg->type_traits == src_reg->type_traits);

    ASSERT_MSG(ecs_is_valid(src_reg, src_ett), "invalid entity");

    cpy_ett = ecs_create(dst_reg);

    for (u32 type_id = 0; type_id < dst_reg->type_count; ++type_id)
        if ((src_mem = ecs_pool_get(src_reg->pools[type_id], src_ett)) != NULL)
            ecs_pool_cpy(dst_reg->pools[type_id], cpy_ett, src_mem);

    return cpy_ett;
}

bool
ecs_is_valid(struct ecs_Registry* reg, ecs_entity_t ett)
{
    ecs_size_t idx = ECS_TO_INDEX(ett);
    return (idx < reg->size) && (reg->entities[idx] == ett);
}

void
ecs_connect(struct ecs_Registry* reg, int event, ecs_size_t type_id, ecs_Callback callback, void* ctx)
{
    ASSERT_MSG(type_id < reg->type_count, "Invalid type");
    ecs_pool_connect(reg->pools[type_id], event, callback, ctx);
}

void
ecs_disconnect(struct ecs_Registry* reg, int event, ecs_size_t type_id, ecs_Callback callback, void* ctx)
{
    ASSERT_MSG(type_id < reg->type_count, "Invalid type");
    ecs_pool_disconnect(reg->pools[event], event, callback, ctx);
}
