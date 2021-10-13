#include "components.h"
#include "ecs.h"
#include "entity.h"

INLINE void
compute_transform_matrix_lower(float x, float y, float sx, float sy, float rotation, mat3 dst)
{
    float angle  = -glm_rad(rotation);
    float cosine = SDL_cosf(angle);
    float sine   = SDL_sinf(angle);
    float sxc    = sx * cosine;
    float syc    = sy * cosine;
    float sxs    = sx * sine;
    float sys    = sy * sine;
    float tx     = x;
    float ty     = y;

    dst[0][0] = sxc;
    dst[0][1] = -sxs;
    dst[0][2] = 0.f;

    dst[1][0] = sys;
    dst[1][1] = syc;
    dst[1][2] = 0.f;

    dst[2][0] = tx;
    dst[2][1] = ty;
    dst[2][2] = 1.f;
}

INLINE void
compute_transform_matrix(const struct TransformComp* transform_comp, mat3 dst)
{
    float x     = transform_comp->position[0];
    float y     = transform_comp->position[1] + transform_comp->position[2];
    float sx    = transform_comp->scale[0];
    float sy    = transform_comp->scale[1];
    float angle = transform_comp->rotation;
    compute_transform_matrix_lower(x, y, sx, sy, angle, dst);
}

void
update_world_transform_matrix_recursive(ecs_Registry* registry,
                                        ecs_entity_t  ett,
                                        mat3          parent_matrix,
                                        bool          parent_dirty,
                                        int           depth)
{
    bool  local_dirty     = parent_dirty | ecs_has(registry, ett, TransformChangedTag);
    vec3* local_transform = ecs_get(registry, ett, LocalTransformMatrixComp)->value;
    vec3* world_transform = ecs_get(registry, ett, WorldTransformMatrixComp)->value;

    if (local_dirty)
    {
        glm_mat3_mul(parent_matrix, local_transform, world_transform);
        ecs_add(registry, ett, WorldTransformMatrixChangedTag);
    }
    ecs_entity_t child = ett_get_first_child(registry, ett);
    while (child != ECS_NULL_ENT)
    {
        update_world_transform_matrix_recursive(registry, child, world_transform, local_dirty, depth + 1);
        child = ett_get_next_sibling(registry, child);
    }
}

void
update_world_transform_matrices(ecs_Registry* registry)
{
    struct ecs_View          view;
    ecs_entity_t             ett;
    struct RelationshipComp* relationship;

    // TODO: place all root entity to the right of the pool so we can ingore checking
    // may be create a binary divisor
    ecs_view_init(&view, registry, { RelationshipComp });
    while (ecs_view_next(&view, &ett, &relationship))
    {
        bool has_parent = relationship->parent != ECS_NULL_ENT;
        bool dirty      = ecs_has(registry, ett, TransformChangedTag);
        if (!has_parent)
        {
            update_world_transform_matrix_recursive(registry, ett, GLM_MAT3_IDENTITY, dirty, 0);
        }
    }
}

void
update_standalone_world_transform_matrices(ecs_Registry* registry)
{
    struct ecs_View view;

    void*        components[3];
    ecs_entity_t ett;

    ecs_view_init(&view,
                  registry,
                  { TransformChangedTag, TransformComp, WorldTransformMatrixComp, ECS_EXCL(RelationshipComp) });

    while (ecs_view_next(&view, &ett, components))
    {
        compute_transform_matrix((struct TransformComp*)components[1],
                                 ((struct WorldTransformMatrixComp*)components[2])->value);
        ecs_add(registry, ett, WorldTransformMatrixChangedTag);
    }
}

void
update_local_transform_matrices(ecs_Registry* registry)
{
    struct ecs_View view;
    ecs_entity_t    ett;
    void*           components[3];

    ecs_view_init(&view, registry, { TransformChangedTag, TransformComp, LocalTransformMatrixComp });
    while (ecs_view_next(&view, &ett, components))
    {
        compute_transform_matrix((struct TransformComp*)components[1],
                                 ((struct LocalTransformMatrixComp*)components[2])->value);
    }
}

void
system_rendering_transform_update(ecs_Registry* registry)
{
    update_local_transform_matrices(registry);
    update_world_transform_matrices(registry);
    update_standalone_world_transform_matrices(registry);
}
