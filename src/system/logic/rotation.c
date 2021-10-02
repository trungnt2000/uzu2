#include "components.h"
#include "ecs.h"

void
system_rotation_update(ecs_Registry* registry, float delta_time)
{
    struct ecs_View       view;
    ecs_entity_t          ett;
    struct RotatorComp*   rotator;
    struct TransformComp* transform;

    void* components[2];
    ecs_view_init(&view, registry, { RotatorComp, TransformComp });
    while (ecs_view_next(&view, &ett, components))
    {
        rotator   = components[0];
        transform = components[1];

        transform->rotation += rotator->speed * delta_time;

        ecs_add_or_set(registry, ett, TransformChangedTag, { 0 });
    }
}
