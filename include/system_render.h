// ngotrung Sun 25 Jul 2021 12:47:50 AM +07
#ifndef SYSTEM_RENDER_H
#define SYSTEM_RENDER_H
#include "components.h"
#include "ecs.h"
#include "graphics.h"

/*
 * NAMING CONVENTION
 * system_<module>_<submodule>_<action>
 */

void system_rendering_sprite_init(struct ecs_Registry* registry);
void system_rendering_sprite_shutdown(void);
void system_rendering_sprite_update(struct ecs_Registry* registry);

void system_rendering_animation_update(struct ecs_Registry* registry, float deltaTime);

void system_rendering_character_animation_update(struct ecs_Registry* registry);

void system_rendering_transform_update(struct ecs_Registry* registry);

#endif // SYSTEM_RENDER_H
