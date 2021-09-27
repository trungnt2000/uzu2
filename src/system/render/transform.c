#include "components.h"
#include "ecs.h"
#include "entity.h"

void
update_world_transform_matrix(ecs_Registry* registry, ecs_entity_t ett, mat3 parent_matrix, bool dirty)
{
    bool  local_dirty     = dirty | ecs_has(registry, ett, TransformChangedTag);
    vec3* local_transform = ecs_get(registry, ett, LocalTransformMatrixComp)->value;
    vec3* world_transform = ecs_get(registry, ett, WorldTransformMatrixComp)->value;

    glm_mat3_mul(parent_matrix, local_transform, world_transform);
    ecs_entity_t child = ett_rs_get_first_child(registry, ett);
    while (child != ECS_NULL_ENT)
    {
        update_world_transform_matrix(registry, child, world_transform, local_dirty);
        child = ett_rs_get_next_sibling(registry, child);
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
            update_world_transform_matrix(registry, ett, GLM_MAT3_IDENTITY, dirty);
        }
    }
}

void
update_local_transform_matrices(ecs_Registry* registry)
{
    struct ecs_View                  view;
    ecs_entity_t                     ett;
    void*                            components[3];
    struct TransformComp*            transform;
    struct LocalTransformMatrixComp* local;

    ecs_view_init(&view, registry, { TransformChangedTag, TransformComp, LocalTransformMatrixComp });
    while (ecs_view_next(&view, &ett, components))
    {
        transform = components[1];
        local     = components[2];

        float angle  = -glm_rad(transform->rotation);
        float cosine = SDL_cosf(angle);
        float sine   = SDL_sinf(angle);
        float sxc    = transform->scale[0] * cosine;
        float syc    = transform->scale[1] * cosine;
        float sxs    = transform->scale[0] * sine;
        float sys    = transform->scale[1] * sine;
        float tx     = transform->position[0];
        float ty     = transform->position[1];

        mat3 matrix = {
            { sxc, -sxs, 0.f },
            { sys, syc, 0.f },
            { tx, ty, 1.f },
        };

        glm_mat3_copy(matrix, local->value);
    }
}

void
system_rendering_transform_update(ecs_Registry* registry)
{
    update_local_transform_matrices(registry);
    update_world_transform_matrices(registry);
}
