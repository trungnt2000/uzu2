// ngotrung Sat 14 Aug 2021 10:01:32 PM +07
#ifndef SYSTEM_LOGIC_H
#define SYSTEM_LOGIC_H

#include "ecs.h"
#include "graphics/view.h"

void system_motion_update(ecs_Registry* registry, float delta_time);

void system_input_update(ecs_Registry* registry, OthoCamera* camera);

void system_controller_system(ecs_Registry* registry, float delta_time);

void system_tag_update(ecs_Registry* registry);

void system_box_collision_init(ecs_Registry* registry);
void system_box_collision_shutdown(void);
void system_box_collision_set_callback(void(void*, ecs_entity_t, ecs_entity_t), void* ctx);

#endif // SYSTEM_LOGIC_H
