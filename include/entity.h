// ngotrung Wed 08 Sep 2021 10:04:01 PM +07
#ifndef TRANSFORM_UTILS_H
#define TRANSFORM_UTILS_H
#include "cglm/cglm.h"
#include "ecs.h"
void ett_tx_set_position(ecs_Registry* registry, ecs_entity_t entity, vec3 position);
void ett_tx_set_rotation(ecs_Registry* registry, ecs_entity_t entity, float angle);
void ett_tx_set_scale(ecs_Registry* registry, ecs_entity_t entity, vec2 scale);
void ett_tx_rotate_by(ecs_Registry* registry, ecs_entity_t entity, float angle);

void         ett_rs_add_child(ecs_Registry* reg, ecs_entity_t ett, ecs_entity_t child);
bool         ett_rs_has_parent(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_rs_get_parent(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_rs_get_first_child(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_rs_get_next_sibling(ecs_Registry* reg, ecs_entity_t ett);
ecs_entity_t ett_rs_get_prev_sibling(ecs_Registry* reg, ecs_entity_t ett);

void ett_set_name_fmt(ecs_Registry* reg, ecs_entity_t ett, const char* restrict fmt, ...);

#endif // TRANSFORM_UTILS_H
