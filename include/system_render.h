// ngotrung Sun 25 Jul 2021 12:47:50 AM +07
#ifndef SYSTEM_RENDER_H
#define SYSTEM_RENDER_H
#include "ecs.h"
#include "font_loader.h"

/*
 * NAMING CONVENTION
 * system_<module>_<submodule>_<action>
 */

void system_rendering_sprite_init(ecs_Registry* registry);
void system_rendering_sprite_fini(void);
void system_rendering_sprite_update(ecs_Registry* registry);

void system_rendering_animation_update(ecs_Registry* registry, float deltaTime);

void system_rendering_transform_update(ecs_Registry* registry);

#endif // SYSTEM_RENDER_H
