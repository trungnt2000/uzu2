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

struct NpcParams
{
    float            x;
    float            y;
    const Animation* animations;
    const char*      commands[MAX_INTERACT_COMMAND];
};

ecs_entity_t spawn_player(ecs_Registry* registry, float x, float y, const struct PlayerData* player_data);
ecs_entity_t spawn_monster(ecs_Registry* registry);
ecs_entity_t spawn_npc(ecs_Registry* registry, const struct NpcParams* params);
ecs_entity_t spawn_weapon(ecs_Registry* registry, const Sprite* sprite);
ecs_entity_t spawn_testing_npc(ecs_Registry* registry, float x, float y);

#endif // ENTITY_FACTORY_H
