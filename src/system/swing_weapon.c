#include "components.h"
#include "ecs.h"
#include "entity.h"

extern void animate_entity_with_affine_anim(ecs_Registry*               registry,
                                            ecs_entity_t                entity,
                                            const struct AffineAnimCmd* anim,
                                            bool                        realative_to_initial_state,
                                            void (*finished_callback)(void*, ecs_Registry*, ecs_entity_t),
                                            void* finished_callback_ctx,
                                            bool  invert);

static const struct AffineAnimCmd s_swing_anim[] = {
    AFFINE_ANIM_CMD(0.f, 0.f, 0.f, 1.f, 1.f, -45.f, 0.02f),
    AFFINE_ANIM_CMD(0.f, 0.f, 0.f, 1.f, 1.f, -60.f, 0.05f),
    AFFINE_ANIM_CMD(0.f, 0.f, 0.f, 1.f, 1.f, 60.f, 0.05f),
    AFFINE_ANIM_CMD_END,
};

static void
finished_callback(UNUSED void* ctx, ecs_Registry* registry, ecs_entity_t ett)
{
    ecs_entity_t parent = ett_get_parent(registry, ett);

    ecs_get(registry, parent, ControllerComp)->in_action    = false;
    ecs_get(registry, parent, FacingDirectionComp)->frezzed = false;
}

void
system_swing_weapon_update(ecs_Registry* registry)
{
    struct ecs_View view;

    union
    {
        void* raw[4];
        struct
        {
            struct ControllerComp*      controller;
            struct SwingWeaponComp*     dummy;
            const struct RefComp*       ref;
            struct FacingDirectionComp* facing_direction;
        };
    } data, *components = &data;

    ecs_entity_t ett;

    ecs_view_init(&view, registry, { ControllerComp, SwingWeaponComp, RefComp, FacingDirectionComp });
    while (ecs_view_next(&view, &ett, data.raw))
    {
        bool invert              = components->facing_direction->value[0] < 0.f;
        bool has_attached_weapon = components->ref->weapon != ECS_NULL_ENT;
        if (!components->controller->in_action && components->controller->action == ACTION_ATTACK &&
            has_attached_weapon)
        {
            components->controller->action        = 0;
            components->controller->in_action     = true;
            components->facing_direction->frezzed = true;

            animate_entity_with_affine_anim(registry,
                                            components->ref->hand,
                                            s_swing_anim,
                                            true,
                                            finished_callback,
                                            NULL,
                                            invert);
        }
    }
}
