// ngotrung Sat 14 Aug 2021 10:01:32 PM +07
#ifndef SYSTEM_LOGIC_H
#define SYSTEM_LOGIC_H

#include "ecs.h"
#include "graphics/view.h"

struct MainContext;

void system_motion_update(ecs_Registry* registry, float delta_time);

void system_input_update(ecs_Registry* registry, OthoCamera* camera);

void system_controller_system(ecs_Registry* registry, float delta_time);

void system_tag_update(ecs_Registry* registry);

void system_box_collision_init(ecs_Registry* registry);
void system_box_collision_shutdown(void);
void system_box_collision_update(ecs_Registry* registry);
void system_box_collision_set_callback(void(void*, ecs_entity_t, ecs_entity_t), void* ctx);

void system_collision_manager_init(ecs_Registry* registry);

void system_rotation_update(ecs_Registry* registry, float delta_time);

void system_swing_weapon_update(ecs_Registry* registry);

void system_affine_animation_update(ecs_Registry* registry, float delta_time);

void system_interactable_entity_update(ecs_Registry* registry);

void system_stats_update(ecs_Registry* registry);

void system_camera_update(ecs_Registry* registry, ecs_entity_t camera_target, struct OthoCamera* camera);
#endif // SYSTEM_LOGIC_H
