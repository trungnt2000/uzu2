#include "components.h"

static void
update_transform(struct AffineAnimComp* anim, ecs_Registry* registry, ecs_entity_t entity)
{
    struct TransformComp*       transform = ecs_get(registry, entity, TransformComp);
    const struct AffineAnimCmd* current   = &anim->cmds[anim->current_frame];

    float s = anim->invert ? -1.f : 1.f;

    transform->scale[0] = current->scale[0];
    transform->scale[1] = current->scale[1];
    if (anim->realative_to_initial_state)
    {
        transform->position[0] = anim->initial_x + current->position[0];
        transform->position[1] = anim->initial_y + current->position[1];

        transform->rotation = anim->initial_rotation + current->rotation * s;
    }
    else
    {
        transform->position[0] = current->position[0];
        transform->position[1] = current->position[1];

        transform->rotation = current->rotation * s;
    }
    ecs_assure(registry, entity, TransformChangedTag);
}

static void
start_animation(struct AffineAnimComp* anim, ecs_Registry* registry, ecs_entity_t entity)
{
    anim->current_frame = 0;
    anim->timer         = anim->cmds[0].duration;
    anim->finished      = anim->cmds[0].type == -1;

    struct TransformComp* transform = ecs_get(registry, entity, TransformComp);

    anim->initial_x        = transform->position[0];
    anim->initial_y        = transform->position[1];
    anim->initial_scale_x  = transform->scale[0];
    anim->initial_scale_y  = transform->scale[1];
    anim->initial_rotation = transform->rotation;

    update_transform(anim, registry, entity);
}

static void
finish_animation(struct AffineAnimComp* anim, ecs_Registry* registry, ecs_entity_t entity)
{
    if (anim->should_reset_to_initial_state)
    {
        struct TransformComp* transform = ecs_get(registry, entity, TransformComp);
        transform->position[0]          = anim->initial_x;
        transform->position[1]          = anim->initial_y;
        transform->scale[0]             = anim->initial_scale_x;
        transform->scale[1]             = anim->initial_scale_y;
        transform->rotation             = anim->initial_rotation;
        ecs_assure(registry, entity, TransformChangedTag);
    }
}

void
tick_animation(struct AffineAnimComp* anim, ecs_Registry* registry, ecs_entity_t entity, float delta_time)
{
    if (anim->finished)
        return;

    anim->timer -= delta_time;
    if (anim->timer <= 0.f)
    {
        anim->current_frame++;
        if (anim->cmds[anim->current_frame].type == -1)
        {
            finish_animation(anim, registry, entity);
            anim->finished = true;
            anim->timer    = 0.f;
        }
        else
        {
            anim->timer = anim->cmds[anim->current_frame].duration + anim->timer;
            update_transform(anim, registry, entity);
        }
    }
}

void
animate_entity_with_affine_anim(ecs_Registry*               registry,
                                ecs_entity_t                entity,
                                const struct AffineAnimCmd* cmds,
                                bool                        realative_to_initial_state,
                                void (*finished_callback)(void*, ecs_Registry*, ecs_entity_t),
                                void* finished_callback_ctx,
                                bool  invert)
{
    struct AffineAnimComp* anim         = ecs_add(registry, entity, AffineAnimComp);
    anim->cmds                          = cmds;
    anim->realative_to_initial_state    = realative_to_initial_state;
    anim->should_reset_to_initial_state = false;
    anim->finished_callback             = finished_callback;
    anim->finished_callback_ctx         = finished_callback_ctx;
    anim->invert                        = invert;
    start_animation(anim, registry, entity);
}

static void
remove_finished_animations(ecs_Registry* registry)
{
    struct AffineAnimComp* anims;
    const ecs_entity_t*    entities;
    ecs_size_t             count;
    ecs_raw(registry, AffineAnimComp, &entities, (void**)&anims, &count);

    for (ecs_size_t i = count; i--;)
    {
        if (anims[i].finished)
        {
            anims[i].finished_callback(anims[i].finished_callback_ctx, registry, entities[i]);
            ecs_rmv(registry, entities[i], AffineAnimComp);
        }
    }
}

static void
update_animations(ecs_Registry* registry, float delta_time)
{

    struct ecs_View view;
    void*           components[2];
    ecs_entity_t    ett;

    ecs_view_init(&view, registry, { AffineAnimComp, TransformComp });
    while (ecs_view_next(&view, &ett, components))
    {
        tick_animation((struct AffineAnimComp*)components[0], registry, ett, delta_time);
    }
}

void
system_affine_animation_update(ecs_Registry* registry, float delta_time)
{
    update_animations(registry, delta_time);
    remove_finished_animations(registry);
}
