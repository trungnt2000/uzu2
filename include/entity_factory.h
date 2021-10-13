// ngotrung Wed 25 Aug 2021 06:15:03 PM +07
#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include "ecs.h"
#include "global.h"
#include "resources.h"

int  entity_factory_init(const Resources* resources);
void entity_factory_shutdown(void);

ecs_entity_t create_lizzard(ecs_Registry* registry, float x, float y);
ecs_entity_t create_anime_sword(ecs_Registry* registry);

ecs_entity_t create_player(ecs_Registry* registry, enum JobId job, float x, float y);
#endif // ENTITY_FACTORY_H
