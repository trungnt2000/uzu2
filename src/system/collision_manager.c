#include "components.h"
#include "ecs.h"
#include "message.h"

#define E(T1, T2, Fn) [ENTITY_TAG_##T1][ENTITY_TAG_##T2] = Fn

static void
player_enemy(ecs_entity_t player, ecs_entity_t enemy)
{
    ems_broadcast(MsgPlayerHitEnemy, { player, enemy });
}

static void
enemy_player(ecs_entity_t enemy, ecs_entity_t player)
{
    ems_broadcast(MsgPlayerHitEnemy, { player, enemy });
}

static void (*const s_handler_table[ENTITY_TAG_CNT][ENTITY_TAG_CNT])(ecs_entity_t, ecs_entity_t) = {
    E(PLAYER, ENEMY, player_enemy),
    E(ENEMY, PLAYER, enemy_player),
};

static void
on_collision(void* ctx, ecs_entity_t e1, ecs_entity_t e2)
{
    ecs_Registry* registry = ctx;

    u32 type1 = ecs_get(registry, e1, EntityTag)->value;
    u32 type2 = ecs_get(registry, e2, EntityTag)->value;

    void (*handle_func)(ecs_entity_t, ecs_entity_t) = s_handler_table[type1][type2];

    if (handle_func != NULL)
    {
        handle_func(e1, e2);
    }
}

extern void system_box_collision_set_callback(void (*)(void*, ecs_entity_t, ecs_entity_t), void* ctx);

void
system_collision_manager_init(ecs_Registry* registry)
{
    system_box_collision_set_callback(on_collision, registry);
}
