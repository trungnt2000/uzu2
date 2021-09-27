// ngotrung Wed 28 Jul 2021 10:49:48 PM +07
#ifndef SYSTEM_RENDER_ANIM_H
#define SYSTEM_RENDER_ANIM_H
#include "components.h"
#include "ecs.h"

void
system_rendering_animation_update(struct ecs_Registry* registry, float delta_time)
{
    struct ecs_View       view;
    ecs_entity_t          ett;
    const struct Sprite*  keyframe;
    void*                 components[2];
    struct SpriteComp*    sprite;
    struct AnimationComp* animation;

    ecs_view_init(&view, registry, { SpriteComp, AnimationComp });
    while (ecs_view_next(&view, &ett, components))
    {
        sprite    = components[0];
        animation = components[1];

        animation->elapsed_time += delta_time;
        keyframe       = animation_get_frame(animation->ref, animation->elapsed_time);
        sprite->sprite = *keyframe;
    }
}
#endif // SYSTEM_RENDER_ANIM_H
