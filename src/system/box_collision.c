
#include "components.h"
#include "toolbox/quad_tree.h"
#include "toolbox/signal.h"

struct EntityPair
{
    ecs_entity_t e1;
    ecs_entity_t e2;
};

struct CollisionCallback
{
    void (*preslove)(ecs_entity_t e1, ecs_entity_t e2);
};

static QuadTree* s_quad_tree;

static struct
{
    void (*fn)(void* ctx, ecs_entity_t e1, ecs_entity_t e2);
    void* ctx;
} s_callback;

static struct
{
    struct EntityPair* items;
    u32                count;
    u32                size;
} s_pbuf;

bool
should_collide(const struct HitBoxComp* h1, const struct HitBoxComp* h2)
{
    return (h1->mask & h2->category) && (h2->mask & h1->category);
}

static struct Box
compute_bounding_box(struct HitBoxComp* hitbox, mat3 local_to_world)
{
    vec3 position = { 0 };
    glm_mat3_mulv(local_to_world, (vec3){ 0.f, 0.f, 1.f }, position);

    struct Box box;
    box.left   = position[0] - hitbox->anchor[0];
    box.top    = position[1] - hitbox->anchor[1];
    box.right  = box.left + hitbox->size[0];
    box.bottom = box.top + hitbox->size[1];

    return box;
}

static void
on_hitbox_added(UNUSED void* ctx, ecs_Registry* registry, ecs_entity_t ett, void* component)
{
    struct HitBoxComp* hitbox = component;
    struct Box box = compute_bounding_box(hitbox, ecs_get(registry, ett, WorldTransformMatrixComp)->value);

    hitbox->proxy_id = quad_tree_create_element(s_quad_tree, box, (void*)ett);
}

static void
on_hitbox_removed(UNUSED void* ctx, UNUSED ecs_Registry* registry, UNUSED ecs_entity_t ett, void* component)
{
    struct HitBoxComp* hitbox = component;

    quad_tree_destroy_element(s_quad_tree, hitbox->proxy_id);
}

INLINE ecs_entity_t
ettmax(ecs_entity_t e1, ecs_entity_t e2)
{
    return e1 > e2 ? e1 : e2;
}

INLINE ecs_entity_t
ettmin(ecs_entity_t e1, ecs_entity_t e2)
{
    return e1 < e2 ? e1 : e2;
}

static void
pbuf_asure_size(u32 min_size)
{
    if (s_pbuf.size < min_size)
    {
        s_pbuf.size  = min_size;
        s_pbuf.items = SDL_realloc(s_pbuf.items, min_size);
    }
}

static void
pbuf_push(ecs_entity_t e1, ecs_entity_t e2)
{
    if (s_pbuf.count == s_pbuf.size)
    {
        pbuf_asure_size(s_pbuf.size * 2);
    }
    s_pbuf.items[s_pbuf.count++] = (struct EntityPair){ ettmax(e1, e2), ettmin(e1, e2) };
}

static void
pbuf_init(u32 size)
{
    s_pbuf.size  = 0;
    s_pbuf.count = 0;
    s_pbuf.items = NULL;
    pbuf_asure_size(size);
}

bool
is_equal_entity_pair(struct EntityPair lhs, struct EntityPair rhs)
{
    return lhs.e1 == rhs.e1 && lhs.e2 == rhs.e2;
}

static int
compare_entity_pair(const void* a, const void* b)
{
    const struct EntityPair* lhs = a;
    const struct EntityPair* rhs = b;
    return ((lhs->e1 < rhs->e1) - (rhs->e1 < lhs->e1)) || ((lhs->e2 < rhs->e2) - (rhs->e2 < lhs->e1));
}

static void
pbuf_cleanup(void)
{
    SDL_free(s_pbuf.items);
    s_pbuf.items = NULL;
}

static u32
remove_duplicate(struct EntityPair* pairs, u32 cnt)
{

    if (cnt == 0 || cnt == 1)
        return cnt;

    u32 j = 0;

    for (u32 i = 0; i < cnt - 1; i++)
        if (!is_equal_entity_pair(pairs[i], pairs[i + 1]))
            pairs[j++] = pairs[i];

    pairs[j++] = pairs[cnt - 1];
    return j;
}

void
system_box_collision_init(ecs_Registry* registry)
{
    s_quad_tree = quad_tree_create(8, (struct Box){ -512, -512, 512, 512 });

    pbuf_init(256);
    ecs_connect(registry, ECS_SIG_ADD, HitBoxComp, on_hitbox_added, NULL);
    ecs_connect(registry, ECS_SIG_RMV, HitBoxComp, on_hitbox_removed, NULL);
}

void
system_box_collision_shutdown()
{
    quad_tree_free_null(s_quad_tree);
    pbuf_cleanup();
}

void
system_box_collision_set_callback(void (*callback)(void*, ecs_entity_t, ecs_entity_t), void* ctx)
{
    s_callback.fn  = callback;
    s_callback.ctx = ctx;
}

struct QueryCallbackArg
{
    int           proxy_id;
    ecs_entity_t  entity;
    ecs_Registry* registry;
};

static bool
query_callback(void* ctx, int element)
{
    struct QueryCallbackArg* arg   = ctx;
    ecs_entity_t             other = (ecs_entity_t)quad_tree_get_user_data(s_quad_tree, element);
    struct HitBoxComp*       h1    = ecs_get(arg->registry, arg->entity, HitBoxComp);
    struct HitBoxComp*       h2    = ecs_get(arg->registry, other, HitBoxComp);
    if (arg->proxy_id != element && should_collide(h1, h2))
    {
        pbuf_push(arg->entity, other);
    }
    return true;
}

void
update_proxies(ecs_Registry* registry)
{
    struct ecs_View view;

    struct HitBoxComp*               hitbox;
    void*                            components[3];
    ecs_entity_t                     ett;
    struct Box                       box;
    struct WorldTransformMatrixComp* world_transform_matrix;

    ecs_view_init(&view, registry, { WorldTransformMatrixChangedTag, HitBoxComp, WorldTransformMatrixComp });
    while (ecs_view_next(&view, &ett, components))
    {
        hitbox                 = components[1];
        world_transform_matrix = components[2];

        box = compute_bounding_box(hitbox, world_transform_matrix->value);

        quad_tree_update_element(s_quad_tree, hitbox->proxy_id, box);
    }
}

void
broad_phase(ecs_Registry* registry)
{
    struct ecs_View view;

    struct HitBoxComp*               hitbox;
    void*                            components[3];
    ecs_entity_t                     ett;
    struct Box                       box;
    struct WorldTransformMatrixComp* world_transform_matrix;

    ecs_view_init(&view, registry, { WorldTransformMatrixChangedTag, HitBoxComp, WorldTransformMatrixComp });

    s_pbuf.count = 0;
    while (ecs_view_next(&view, &ett, components))
    {
        hitbox                 = components[1];
        world_transform_matrix = components[2];

        box = compute_bounding_box(hitbox, world_transform_matrix->value);

        quad_tree_query(
            s_quad_tree,
            box,
            query_callback,
            &(struct QueryCallbackArg){ .entity = ett, .proxy_id = hitbox->proxy_id, .registry = registry });
    }
    SDL_qsort(s_pbuf.items, s_pbuf.count, sizeof(struct EntityPair), compare_entity_pair);
    s_pbuf.count = remove_duplicate(s_pbuf.items, s_pbuf.count);
}

static struct Box
compute_entity_bounds(ecs_Registry* registry, ecs_entity_t entity)
{
    vec3*              local_to_world = ecs_get(registry, entity, WorldTransformMatrixComp)->value;
    struct HitBoxComp* hitbox         = ecs_get(registry, entity, HitBoxComp);

    return compute_bounding_box(hitbox, local_to_world);
}

extern bool check_box_overlaps(const struct Box* b1, const struct Box* b2);

static bool
is_collide(ecs_Registry* registry, ecs_entity_t e1, ecs_entity_t e2)
{

    struct Box box1 = compute_entity_bounds(registry, e1);
    struct Box box2 = compute_entity_bounds(registry, e2);
    return check_box_overlaps(&box1, &box2);
}

INLINE void
invoke_callback(ecs_entity_t e1, ecs_entity_t e2)
{
    if (s_callback.fn != NULL)
    {
        s_callback.fn(s_callback.ctx, e1, e2);
    }
}

static void
narrow_phase(ecs_Registry* registry, struct EntityPair* pairs, u32 count)
{
    for (u32 i = 0; i < count; ++i)
    {
        if (is_collide(registry, pairs[i].e1, pairs[i].e2))
        {
            invoke_callback(pairs[i].e1, pairs[i].e2);
        }
    }
}

void
system_box_collision_update(ecs_Registry* registry)
{
    update_proxies(registry);

    broad_phase(registry);
    narrow_phase(registry, s_pbuf.items, s_pbuf.count);
}

struct TestBoxOverlapQueryCBArgs
{
    ecs_entity_t* ebuf;
    u32           max;
    u32           mask;
    u32*          count_ret;
    ecs_Registry* registry;
    struct Box    box;
};

static bool
test_box_query_callback(void* ctx, int element_id)
{
    struct TestBoxOverlapQueryCBArgs* args = ctx;
    if (*args->count_ret == args->max)
        return false;

    ecs_entity_t       ett            = (ecs_entity_t)quad_tree_get_user_data(s_quad_tree, element_id);
    struct HitBoxComp* hitbox         = ecs_get(args->registry, ett, HitBoxComp);
    vec3*              local_to_world = ecs_get(args->registry, ett, WorldTransformMatrixComp)->value;
    struct Box         box            = compute_bounding_box(hitbox, local_to_world);

    if (args->mask & hitbox->category && check_box_overlaps(&box, &args->box))
        args->ebuf[(*args->count_ret)++] = ett;

    return true;
}

void
test_box_overlap(ecs_Registry* registry, struct Box box, ecs_entity_t* ebuf, u32 max, u32* count_ret, u32 mask)
{
    struct TestBoxOverlapQueryCBArgs callback_args = { .ebuf      = ebuf,
                                                       .max       = max,
                                                       .count_ret = count_ret,
                                                       .mask      = mask,
                                                       .box       = box,
                                                       .registry  = registry };

    quad_tree_query(s_quad_tree, box, test_box_query_callback, &callback_args);
}


//


