#include "ecs_priv.h"

static bool
allof(struct ecs_Pool** pools, ecs_size_t count, ecs_entity_t ett)
{
    for (int i = 0; i < count; ++i)
    {
        if (!ecs_pool_contains(pools[i], ett))
        {
            return false;
        }
    }
    return true;
}

static bool
noneof(struct ecs_Pool** pools, ecs_size_t count, ecs_entity_t ett)
{
    for (int i = 0; i < count; ++i)
    {
        if (ecs_pool_contains(pools[i], ett))
        {
            return false;
        }
    }
    return true;
}

static bool
is_entity_belong_to_group(struct ecs_Group* group, ecs_entity_t ett)
{
    return allof(group->own_pools, group->own_count, ett) && noneof(group->excl_pools, group->excl_count, ett);
}

bool
ecs_group_contains(struct ecs_Group* g, ecs_entity_t ett)
{
    // this group contain given entity when first pool contains the entity
    // and it index must smaller than group size
    ecs_size_t idx = ecs_pool_index(g->own_pools[0], ett);
    return idx != ECS_NULL_IDX && idx < g->size;
}

static ecs_size_t
add_hook_impl(void* ctx, struct ecs_Pool* pool, ecs_entity_t ett, ecs_size_t idx)
{
    struct ecs_Group* group = ctx;
    if (is_entity_belong_to_group(group, ett))
    {
        // swap given entity to right after last entity in group
        ecs_size_t        new_index = group->size;
        struct ecs_Pool** pools     = group->own_pools;
        for (ecs_size_t i = 0; i < group->own_count; ++i)
        {
            ecs_entity_t lhs = ett;
            ecs_entity_t rhs = pools[i]->entities[new_index];
            if (lhs != rhs)
            {
                ecs_pool_swp(pools[i], lhs, rhs);
            }
        }
        idx = new_index;

        group->size++;
        if (group->child != NULL)
        {
            idx = add_hook_impl(group->child, pool, ett, idx);
        }
    }
    return idx;
}

static ecs_size_t
rmv_hook_impl(void* ctx, struct ecs_Pool* pool, ecs_entity_t ett, ecs_size_t idx)
{
    struct ecs_Group* group = ctx;
    if (ecs_group_contains(group, ett))
    {
        if (group->child != NULL)
        {
            idx = rmv_hook_impl(group, pool, ett, idx);
        }
        // move given entity to the end of this group
        // all entities are inside same group will have same index
        ecs_size_t        last_index = group->size - 1;
        ecs_entity_t      lhs        = ett;
        ecs_entity_t      rhs        = pool->entities[last_index];
        struct ecs_Pool** pools      = group->own_pools;
        if (lhs != rhs)
        {
            for (ecs_size_t i = 0; i < group->own_count; ++i)
            {
                ecs_pool_swp(pools[i], lhs, rhs);
            }
        }
        group->size--;
        idx = last_index;
    }
    return idx;
}

static bool
has_pool(struct ecs_Pool** pools, ecs_size_t cnt, struct ecs_Pool* p)
{
    for (int i = 0; i < cnt; ++i)
        if (pools[i] == p)
            return true;

    return false;
}

/* Check whether or not self group is child of other group */
static bool
is_child_of(struct ecs_Group* self, struct ecs_Group* other)
{
    // this group must have more types to be child of other group
    if (self->own_count < other->own_count)
        return false;

    // and all types that parent group has this group must have too
    for (int i = 0; i < other->own_count; ++i)
        if (!has_pool(self->own_pools, self->own_count, other->own_pools[i]))
            return false;

    // and it must not contain any excluded types of other group
    for (int i = 0; i < other->excl_count; ++i)
        if (has_pool(self->own_pools, self->own_count, other->excl_pools[i]))
            return false;

    return true;
}

/* Check whether or not self group is parent of other group */
static bool
is_parent_of(struct ecs_Group* self, struct ecs_Group* other)
{

    // to be parent of other group this group must have fewer types than
    // other group
    if (self->own_count > other->own_count)
        return false;

    // and all types it owns that group must have too
    for (int i = 0; i < self->own_count; ++i)
        if (!has_pool(other->own_pools, other->own_count, self->own_pools[i]))
            return false;

    // and that group must not contain any excluded types of this group
    for (int i = 0; i < self->excl_count; ++i)
        if (has_pool(other->own_pools, other->own_count, self->excl_pools[i]))
            return false;

    return true;
}

void
ecs_add_group(struct ecs_Registry* registry, struct ecs_Group* group)
{
    for (int i = 0; i < registry->group_count; ++i)
    {
        struct ecs_Group* other_group = registry->groups[i];
        if (is_child_of(group, other_group))
        {
            // walk down to find best place where no more group in this chain or
            // given this group is parent of next group
            while (other_group->child != NULL && is_child_of(group, other_group->child))
                other_group = other_group->child;

            // double check in case this group is not parent of next in case the
            // next group has excluded types
            if (other_group->child == NULL || is_parent_of(group, other_group->child))
            {
                group->child       = other_group->child;
                other_group->child = group;

                group->size = group->child != NULL ? group->child->size : 0;
                return;
            }
        }
        else if (is_parent_of(group, other_group))
        {
            // make it become toplevel group
            registry->groups[i] = group;

            // initialize
            group->child = other_group;

            group->size = other_group->size;
            return;
        }
    }

    bool sortable = true;
    for (int i = 0; i < group->own_count; ++i)
    {
        if (!ecs_pool_sortable(group->own_pools[i]))
        {
            sortable = false;
            break;
        }
    }

    if (sortable)
    {
        registry->group_count += 1;
        registry->groups = SDL_realloc(registry->groups, sizeof(struct ecs_Group*) * registry->group_count);
        registry->groups[registry->group_count - 1] = group;

        for (int i = 0; i < group->own_count; ++i)
        {
            ecs_pool_set_hook(group->own_pools[i],
                              (struct ecs_Hook){ add_hook_impl, rmv_hook_impl, .ctx = group });
        }

        return;
    }
    ASSERT_MSG(0, "Could not create group");
}

struct ecs_Group*
_ecs_create_group(ecs_Registry* registry, ecs_size_t* Ts, ecs_size_t Tc)
{
    ecs_Group* group = ecs_group_create(registry->pools, Ts, Tc);
    ecs_add_group(registry, group);
    return group;
}

struct ecs_Group*
ecs_group_create(struct ecs_Pool* const* master_pools, ecs_size_t* Ts, ecs_size_t Tc)
{
    struct ecs_Group* group = SDL_malloc(sizeof(struct ecs_Group));

    struct ecs_Pool* own[64];
    struct ecs_Pool* excl[64];
    struct ecs_Pool* shar[64];

    // a small shortcut
    struct ecs_Pool* const* pools = master_pools;

    group->excl_count = 0;
    group->own_count  = 0;
    group->shar_count = 0;
    group->size       = 0;
    group->child      = NULL;

    for (int i = 0; i < Tc; ++i)
    {
        if (Ts[i] & ECS_EXCL_MASK)
        {
            excl[group->excl_count++] = pools[Ts[i] & ECS_TYPE_MASK];
        }
        else if (Ts[i] & ECS_SHAR_MASK)
        {
            shar[group->shar_count++] = pools[Ts[i] & ECS_TYPE_MASK];
        }
        else
        {
            own[group->own_count++] = pools[Ts[i] & ECS_TYPE_MASK];
        }
    }

    group->excl_pools = SDL_malloc(sizeof(void*) * group->excl_count);
    group->own_pools  = SDL_malloc(sizeof(void*) * group->own_count);
    group->shar_pools = SDL_malloc(sizeof(void*) * group->shar_count);

    SDL_memcpy(group->excl_pools, excl, group->excl_count * sizeof(void*));
    SDL_memcpy(group->own_pools, own, group->own_count * sizeof(void*));
    SDL_memcpy(group->shar_pools, shar, group->shar_count * sizeof(void*));

    group->size  = 0;
    group->child = NULL;

    return (group);
}

void
ecs_group_free(struct ecs_Group* group)
{
    if (!group)
        return;

    for (int i = 0; i < group->own_count; ++i)
    {
        ecs_pool_set_hook(group->own_pools[i], (struct ecs_Hook){ 0 });
    }
    SDL_free(group->excl_pools);
    SDL_free(group->own_pools);
    SDL_free(group->shar_pools);

    SDL_free(group);
}

void*
ecs_group_data_begin(struct ecs_Group* group, ecs_size_t index)
{
    ASSERT_MSG(index < group->own_count, "invalid component type index");

    return ecs_pool_data_begin(group->own_pools[index]);
}

const ecs_entity_t*
ecs_group_ett_begin(struct ecs_Group* group)
{
    return ecs_pool_ett_begin(group->own_pools[0]);
}

ecs_size_t
ecs_group_size(struct ecs_Group* group)
{
    return group->size;
}
