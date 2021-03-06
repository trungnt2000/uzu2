#include "ecs_priv.h"

#define INDEX_MASK 0xfff

static bool
are_all_pool_contain_given_entity(struct ecs_Pool** pool_list, u32 pool_count, ecs_entity_t entity)
{
    for (u32 i = 0; i < pool_count; ++i)
    {
        if (!ecs_pool_contains(pool_list[i], entity))
        {
            return false;
        }
    }
    return true;
}

static bool
are_any_pool_contain_given_entity(struct ecs_Pool** pool_list, u32 pool_count, ecs_entity_t entity)
{
    for (u32 i = 0; i < pool_count; ++i)
        if (ecs_pool_contains(pool_list[i], entity))
            return true;
    return false;
}

static void
get_components_form_pool_list(struct ecs_Pool** pool_list,
                              u32               pool_count,
                              ecs_entity_t      entity,
                              void**            components)
{
    for (u32 i = 0; i < pool_count; ++i)
    {
        components[i] = ecs_pool_get(pool_list[i], entity);
    }
}

void
ecs_view_for_each(struct ecs_View* view, void (*fn)(void*, ecs_Registry*, ecs_entity_t, void* as_array))
{
    void** components = SDL_stack_alloc(void*, view->reqd_count);

    for (const ecs_entity_t* iter = view->ett_iter; iter != view->ett_end; ++iter)
    {
        if (are_all_pool_contain_given_entity(view->required_pools, view->reqd_count, *iter) &&
            are_any_pool_contain_given_entity(view->excluded_pools, view->excl_count, *iter))
        {
            get_components_form_pool_list(view->required_pools, view->reqd_count, *iter, components);
            fn(NULL, NULL, *iter, components);
        }
    }

    SDL_stack_free(component);
}

void
_ecs_view_init(struct ecs_View* view, ecs_Registry* reg, ecs_size_t* types, u32 cnt)
{
    view->excl_count        = 0;
    view->reqd_count        = 0;
    struct ecs_Pool** pools = reg->pools;
    for (u32 i = 0; i < cnt; ++i)
    {
        if (types[i] & ECS_EXCL_MASK)
        {
            ASSERT(view->excl_count < ECS_VIEW_MAX_COMP_CNT);
            view->excluded_pools[view->excl_count++] = pools[types[i] & INDEX_MASK];
        }
        else
        {
            ASSERT(view->reqd_count < ECS_VIEW_MAX_COMP_CNT);
            view->required_pools[view->reqd_count++] = pools[types[i] & INDEX_MASK];
        }
    }

    ASSERT(view->reqd_count > 0 && "create view need at least 1 component");

    view->fast_path                = 0;
    struct ecs_Pool* smallest_pool = view->required_pools[0];
    for (u32 i = 1; i < view->reqd_count; ++i)
    {
        if (view->required_pools[i]->count < view->required_pools[view->fast_path]->count)
        {
            view->fast_path = i;
            smallest_pool   = view->required_pools[i];
        }
    }

    /* interator begin at the last element in packed array */
    view->data_iter = ecs_pool_data_rbegin(smallest_pool);
    view->data_end  = ecs_pool_data_rend(smallest_pool);
    view->ett_iter  = ecs_pool_ett_rbegin(smallest_pool);
    view->ett_end   = ecs_pool_ett_rend(smallest_pool);

    view->tsiz = smallest_pool->traits.size;
}

bool
_ecs_view_next(struct ecs_View* v, ecs_entity_t* ett, void* comps[])
{
    if (v->ett_iter == v->ett_end)
        return false;

    bool              match = false;
    bool              has_excluded_component;
    u8*               data_pos;
    struct ecs_Pool** excluded_pools = v->excluded_pools;
    struct ecs_Pool** required_pools = v->required_pools;

    /* use backward iteration to freely destroy entity */
    while (!match && v->ett_iter != v->ett_end)
    {
        /* save current position of fast path */
        data_pos = v->data_iter;

        /* move iterator */
        v->data_iter = (u8*)v->data_iter - v->tsiz;
        *ett         = *(v->ett_iter--);

        /* check whether or not entity has excluded component */
        // TODO: any_of function
        has_excluded_component = false;
        for (u32 i = 0; i < v->excl_count; ++i)
        {
            if (ecs_pool_contains(excluded_pools[i], *ett))
            {
                has_excluded_component = true;
                break;
            }
        }

        if (!has_excluded_component)
        {
            match = true;
            for (u32 i = 0; i < v->reqd_count; ++i)
            {
                if (i == v->fast_path)
                {
                    comps[i] = data_pos;
                }
                else
                {
                    comps[i] = ecs_pool_get(required_pools[i], *ett);
                    if (comps[i] == NULL)
                    {
                        match = false;
                        break;
                    }
                }
            }
        }
    }
    return match;
}
