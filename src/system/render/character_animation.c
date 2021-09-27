#include "components.h"
#include "ecs.h"

#define VEC2_MAG2(v) ((v)[0] * (v[0]) + (v)[1] * (v)[1])
void
system_rendering_character_animation_update(ecs_Registry* registry)
{
    struct ecs_View view;
    ecs_entity_t    ett;

    void*                                    components[6];
    struct CharacterAnimationControllerComp* controller;
    struct AnimationPoolComp*                pool;
    struct AnimationComp*                    animation;
    struct VelocityComp*                     velocity;
    struct FacingDirectionComp*              facing_direction;
    struct SpriteComp*                       sprite;

    float                   magnitude2;
    CharacterAnimationState next_state;

    ecs_view_init(&view,
                  registry,
                  { CharacterAnimationControllerComp,
                    AnimationComp,
                    AnimationPoolComp,
                    VelocityComp,
                    FacingDirectionComp,
                    SpriteComp });

    while (ecs_view_next(&view, &ett, components))
    {
        controller       = components[0];
        animation        = components[1];
        pool             = components[2];
        velocity         = components[3];
        facing_direction = components[4];
        sprite           = components[5];

        magnitude2 = VEC2_MAG2(velocity->value);

        if (magnitude2 > 0.1f)
            next_state = CHARACTER_ANIMATION_WALK;
        else
            next_state = CHARACTER_ANIMATION_IDLE;

        if (next_state != controller->state)
        {
            controller->state       = next_state;
            animation->ref          = &pool->animations[next_state];
            animation->elapsed_time = 0.f;
        }

        int sign = signf(facing_direction->value[0]);
        if (sign != 0)
        {
            sprite->hori_flip = sign < 0 ? true : false;
        }
    }
}
