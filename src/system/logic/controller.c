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
    vec2                   force;

    ecs_view_init(&view, registry, { ControllerComp, VelocityComp, SpeedComp });
    while (ecs_view_next(&view, &ett, components))
    {
        controller = components[0];
        velocity   = components[1];
        speed      = components[2];

        glm_vec2_scale_as(controller->desired_direction, speed->value * delta_time * 25.f, force);
        glm_vec2_add(velocity->value, force, velocity->value);

        if (glm_vec2_norm2(velocity->value) > (speed->value * speed->value))
        {
            glm_vec2_scale_as(velocity->value, speed->value, velocity->value);
        }
        glm_vec2_zero(controller->desired_direction);
    }
}
