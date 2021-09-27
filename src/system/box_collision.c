
#include "components.h"
#include "toolbox/quad_tree.h"
#include "toolbox/signal.h"

struct EntityPair
{
    ecs_entity_t e1;
    ecs_entity_t e2;
};

static QuadTree* s_quad_tree;

struct
{
    void (*fn)(void* ctx, ecs_entity_t e1, ecs_entity_t e2);
    void* ctx;
} s_callback;

static struct
{
    struct EntityPair* items;
    u32                count;
    u32                size;
} s_pair_list;

static struct Box
compute_bounding_box(struct HitBoxComp* hitbox, mat3 local_to_world)
{
    vec2 position = { 0.f, 0.f };
    glm_mat3_mulv(local_to_world, (vec2){ 0 }, position);

    return (struct Box){
        .left   = position[0] - hitbox->anchor[0],
        .top    = position[1] - hitbox->anchor[1],
        .right  = hitbox->size[0],
        .bottom = hitbox->size[1],
    };
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
maxe(ecs_entity_t e1, ecs_entity_t e2)
{
    return e1 > e2 ? e1 : e2;
}

INLINE ecs_entity_t
mine(ecs_entity_t e1, ecs_entity_t e2)
{
    return e1 < e2 ? e1 : e2;
}

static void
push_to_pair_list(ecs_entity_t e1, ecs_entity_t e2)
{
    if (s_pair_list.count == s_pair_list.size)
    {
        s_pair_list.size  = s_pair_list.size * 2;
        s_pair_list.items = SDL_realloc(s_pair_list.items, s_pair_list.size);
    }
    s_pair_list.items[s_pair_list.count++] = (struct EntityPair){ maxe(e1, e2), mine(e1, e2) };
}

bool
check_entity_pair_equality(struct EntityPair lhs, struct EntityPair rhs)
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

static u32
remove_duplicate(struct EntityPair* pairs, u32 cnt)
{

    if (cnt == 0 || cnt == 1)
        return cnt;

    u32 j = 0;

    for (u32 i = 0; i < cnt - 1; i++)
        if (!check_entity_pair_equality(pairs[i], pairs[i + 1]))
            pairs[j++] = pairs[i];

    pairs[j++] = pairs[cnt - 1];
    return j;
}

void
system_box_collision_init(ecs_Registry* registry)
{
    s_quad_tree       = quad_tree_create(8, (struct Box){ 0, 0, 512, 512 });
    s_pair_list.count = 0;
    s_pair_list.size  = 256;
    s_pair_list.items = SDL_malloc((sizeof *s_pair_list.items) * s_pair_list.size);

    ecs_connect(registry, ECS_SIG_ADD, HitBoxComp, on_hitbox_added, NULL);
    ecs_connect(registry, ECS_SIG_RMV, HitBoxComp, on_hitbox_removed, NULL);
}

void
system_box_collision_shutdown()
{
}

void
system_box_collision_set_callback(void (*callback)(void*, ecs_entity_t, ecs_entity_t), void* ctx)
{
    s_callback.fn  = callback;
    s_callback.ctx = ctx;
}

struct QueryCallbackArg
{
    int          proxy_id;
    ecs_entity_t entity;
};

static bool
query_callback(void* ctx, int element)
{
    struct QueryCallbackArg* arg = ctx;
    if (arg->proxy_id != element)
    {
        push_to_pair_list(arg->entity, (ecs_entity_t)quad_tree_get_user_data(s_quad_tree, element));
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

    ecs_view_init(&view, registry, { TransformChangedTag, HitBoxComp, WorldTransformMatrixComp });
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

    ecs_view_init(&view, registry, { TransformChangedTag, HitBoxComp, WorldTransformMatrixComp });

    while (ecs_view_next(&view, &ett, components))
    {
        hitbox                 = components[1];
        world_transform_matrix = components[2];

        box = compute_bounding_box(hitbox, world_transform_matrix->value);

        quad_tree_query(s_quad_tree,
                        box,
                        query_callback,
                        &(struct QueryCallbackArg){ .entity = ett, .proxy_id = hitbox->proxy_id });
    }
    SDL_qsort(s_pair_list.items, s_pair_list.count, sizeof(struct EntityPair), compare_entity_pair);
    s_pair_list.count = remove_duplicate(s_pair_list.items, s_pair_list.count);
}

void
system_box_collision_update(ecs_Registry* registry)
{
    update_proxies(registry);
    broad_phase(registry);

    if (s_callback.fn != NULL)
    {
        for (u32 i = 0; i < s_pair_list.count; ++i)
            s_callback.fn(s_callback.ctx, s_pair_list.items[i].e1, s_pair_list.items[i].e2);
    }
}
