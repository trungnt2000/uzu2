#include "components.h"
#include "ecs.h"

void
system_motion_update(ecs_Registry* registry, float delta_time)
{
    struct ecs_View       view;
    ecs_entity_t          ett;
    float*                pos;
    const float*          vel;
    struct VelocityComp*  velocity;
    struct TransformComp* transform;
    void*                 components[2];

    ecs_view_init(&view, registry, { TransformComp, VelocityComp });

    while (ecs_view_next(&view, &ett, components))
    {
        transform = components[0];
        velocity  = components[1];
        pos       = transform->position;
        vel       = velocity->value;

        pos[0] += vel[0] * delta_time;
        pos[1] += vel[1] * delta_time;

        // tag this entity that it have changed position
        ecs_add_or_set(registry, ett, TransformChangedTag, { 0 });
    }
}
