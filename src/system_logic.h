//ngotrung Sat 14 Aug 2021 10:01:32 PM +07
#ifndef SYSTEM_LOGIC_H
#define SYSTEM_LOGIC_H
#include "ecs.h"

void system_motion_update(ecs_Registry* registry, float deltaTime);

void system_input_update(ecs_Registry* registry, float deltaTime);

void system_pawn_update(ecs_Registry* registry);
#endif //SYSTEM_LOGIC_H
