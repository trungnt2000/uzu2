#include "ecs_priv.h"

#define INDEX_MASK 0xfff

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
    bool              is_excluded;
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
        is_excluded = false;
        for (u32 i = 0; i < v->excl_count; ++i)
        {
            if (ecs_pool_contains(excluded_pools[i], *ett))
            {
                is_excluded = true;
                break;
            }
        }

        if (!is_excluded)
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
