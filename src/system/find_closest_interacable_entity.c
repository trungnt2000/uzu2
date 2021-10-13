
#include "components.h"
#include "ecs.h"
#include "entity.h"
#include "scn_main.h"
#include "toolbox/quad_tree.h"

#define MAX_DISTANCE_TO_PLAYER 100.f

extern void
test_box_overlap(ecs_Registry* registry, struct Box box, ecs_entity_t* ebuf, u32 max, u32* count_ret, u32 mask);

ecs_entity_t
get_closest(ecs_Registry* registry, ecs_entity_t* entities, u32 count, vec2 player_position)
{
    if (count == 0)
        return ECS_NULL_ENT;
    if (count == 1)
        return entities[0];

    u32 min = 0;

    vec2  entity_position;
    float mindist2 = MAX_DISTANCE_TO_PLAYER * MAX_DISTANCE_TO_PLAYER;

    for (u32 i = 1; i < count; ++i)
    {
        ett_get_world_position(registry, entities[i], entity_position);
        float dist2 = glm_vec2_distance2(entity_position, player_position);

        if (dist2 < mindist2)
        {
            mindist2 = dist2;
            min      = i;
        }
    }

    return entities[min];
}

void
system_find_closest_interacable_entity(ecs_Registry* registry, struct MainState* main_state)
{

    vec2 player_position;
    vec2 current_object_position;

    ett_get_world_position(registry, main_state->player, player_position);
    ett_get_world_position(registry, main_state->current_object, current_object_position);

    float distance_to_player2 = glm_vec2_distance2(player_position, current_object_position);

    bool should_update_current_object = main_state->current_object == ECS_NULL_ENT ||
                                        distance_to_player2 > (MAX_DISTANCE_TO_PLAYER * MAX_DISTANCE_TO_PLAYER);

    if (should_update_current_object)
    {
        ecs_entity_t near_by_interactable_entities[10];
        u32          count;
        struct Box   box = {
            .left   = player_position[0] - MAX_DISTANCE_TO_PLAYER,
            .right  = player_position[0] + MAX_DISTANCE_TO_PLAYER,
            .bottom = player_position[1] + MAX_DISTANCE_TO_PLAYER,
            .top    = player_position[1] - MAX_DISTANCE_TO_PLAYER,
        };
        test_box_overlap(registry, box, near_by_interactable_entities, 10, &count, INTERACTABLE_MASK);
    }
}
