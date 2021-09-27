#include "components.h"
#include "ecs.h"

static void
remove_trasform_changed_tag(ecs_Registry* registry)
{
    struct ecs_View view;
    ecs_entity_t    entity;
    void*           components[1];

    ecs_view_init(&view, registry, { TransformChangedTag });
    while (ecs_view_next(&view, &entity, components))
    {
        ecs_rmv(registry, entity, TransformChangedTag);
    }
}

void
system_tag_update(ecs_Registry* registry)
{
    remove_trasform_changed_tag(registry);
}
