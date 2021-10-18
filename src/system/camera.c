#include "components.h"
#include "entity.h"
#include "scn_main.h"

void
system_camera_update(ecs_Registry* registry, ecs_entity_t player, struct OthoCamera* camera)
{
    vec2 player_position;
    vec3 camera_position;

    ett_get_world_position(registry, player, player_position);

    glm_vec3_copy(camera->position, camera_position);

    camera_position[0] = lerpf(camera_position[0], player_position[0], 0.1f);
    camera_position[1] = lerpf(camera_position[1], player_position[1], 0.1f);


    otho_camera_set_position(camera, camera_position);
}
