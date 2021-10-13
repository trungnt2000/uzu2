#include "components.h"
#include "ecs.h"

void
system_motion_update(ecs_Registry* registry, float delta_time)
{
    struct ecs_View view;
    ecs_entity_t    ett;
    float*          pos;
    float*          vel;
    void*           components[2];

    ecs_view_init(&view, registry, { TransformComp, VelocityComp });

    while (ecs_view_next(&view, &ett, components))
    {
        pos                   = ((struct TransformComp*)components[0])->position;
        vel                   = ((struct VelocityComp*)components[1])->value;
        float slowdown_factor = ((struct VelocityComp*)components[1])->slowdown_factor;

        float dx = vel[0] * delta_time;
        float dy = vel[1] * delta_time;

        pos[0] += dx;
        pos[1] += dy;

        vel[0] *= (1.f - slowdown_factor);
        vel[1] *= (1.f - slowdown_factor);

        // tag this entity that it have changed position
        if (absf(dx) > 0.01f || absf(dy) > 0.01f)
        {
            ecs_assure(registry, ett, TransformChangedTag);
        }
    }
}
