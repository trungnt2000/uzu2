#include "components.h"
#include "ecs.h"

void
system_controller_system(ecs_Registry* registry, UNUSED float delta_time)
{
    struct ecs_View        view;
    ecs_entity_t           ett;
    void*                  components[3];
    struct VelocityComp*   velocity;
    struct ControllerComp* controller;
    struct SpeedComp*      speed;
    vec2                   normalized_direction;

    ecs_view_init(&view, registry, { ControllerComp, VelocityComp, SpeedComp });
    while (ecs_view_next(&view, &ett, components))
    {
        controller = components[0];
        velocity   = components[1];
        speed      = components[2];

        glm_vec2_normalize_to(controller->desired_direction, normalized_direction);
        glm_vec2_scale(normalized_direction, speed->value, velocity->value);
        glm_vec2_zero(controller->desired_direction);
    }
}
