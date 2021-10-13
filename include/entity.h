// ngotrung Wed 08 Sep 2021 10:04:01 PM +07
#ifndef TRANSFORM_UTILS_H
#define TRANSFORM_UTILS_H
#include "cglm/cglm.h"
#include "components.h"
#include "ecs.h"
void ett_set_position(ecs_Registry* registry, ecs_entity_t entity, vec3 position);
void ett_set_rotation(ecs_Registry* registry, ecs_entity_t entity, float angle);
void ett_set_scale(ecs_Registry* registry, ecs_entity_t entity, vec2 scale);
void ett_rotate_by(ecs_Registry* registry, ecs_entity_t entity, float angle);

ecs_entity_t ett_add_child(ecs_Registry* reg, ecs_entity_t ett, ecs_entity_t child);
bool         ett_has_parent(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_get_parent(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_get_first_child(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_get_next_sibling(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_get_prev_sibling(ecs_Registry* reg, ecs_entity_t ett);

void ett_set_name_fmt(ecs_Registry* reg, ecs_entity_t ett, const char* restrict fmt, ...);

const char* ett_get_name(ecs_Registry* reg, ecs_entity_t ett);

static inline void
ett_get_world_position(ecs_Registry* reg, ecs_entity_t ett, vec2 world_position_return)
{
    vec3  tmp            = { 0 };
    vec3  in             = { 0.f, 0.f, 1.f };
    vec3* local_to_world = ecs_get(reg, ett, WorldTransformMatrixComp)->value;
    glm_mat3_mulv(local_to_world, in, tmp);
    glm_vec2_copy(tmp, world_position_return);
}

void ett_attach_weapon(ecs_Registry* registry, ecs_entity_t entity, ecs_entity_t weapon);

#endif // TRANSFORM_UTILS_H
