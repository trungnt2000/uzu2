#include "input.h"
#include "components.h"
#include "ecs.h"
#include "entity.h"
#include "scn_main.h"
#include "ui/menu.h"

static void
get_mouse_position(OthoCamera* view, vec2 position_return)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    otho_camera_to_view_coords(view, (vec2){ (float)(mx / 4), (float)(my / 4) }, position_return);
}

void
sync_facing_direction(ecs_Registry* registry, OthoCamera* camera)
{
    struct ecs_View view;
    ecs_entity_t    ett;

    void*                       components[2];
    struct FacingDirectionComp* facing_direction;
    struct TransformComp*       transform;
    vec2                        mouse_position;
    vec2                        to_mouse;

    get_mouse_position(camera, mouse_position);
    ecs_view_init(&view, registry, { FacingDirectionComp, TransformComp, InputComp });
    while (ecs_view_next(&view, &ett, components))
    {
        facing_direction = components[0];
        transform        = components[1];
        if (!facing_direction->frezzed)
        {
            glm_vec2_sub(mouse_position, transform->position, to_mouse);
            glm_vec2_normalize_to(to_mouse, facing_direction->value);
        }
    }
}

void
interact_callback(void* ctx, const char* option, u32 index)
{
}

static void
update_desired_direction(ecs_Registry* registry)
{
    struct ecs_View        view;
    ecs_entity_t           ett;
    struct ControllerComp* controller;
    void*                  components[2];

    ecs_view_init(&view, registry, { ControllerComp, InputComp });
    while (ecs_view_next(&view, &ett, components))
    {
        controller = components[0];
        glm_vec2_zero(controller->desired_direction);
        if (button_pressed(BTN_LEFT))
        {
            controller->desired_direction[0] -= 1.f;
        }

        if (button_pressed(BTN_RIGHT))
        {
            controller->desired_direction[0] += 1.f;
        }

        if (button_pressed(BTN_UP))
        {
            controller->desired_direction[1] -= 1.f;
        }

        if (button_pressed(BTN_DOWN))
        {
            controller->desired_direction[1] += 1.f;
        }

        if (mouse_button_just_pressed(SDL_BUTTON_LEFT) && !controller->in_action)
        {
            controller->action = ACTION_ATTACK;
        }

        if (button_pressed(BTN_INTERACT))
        {
            ecs_entity_t current_object = g_main_ctx.current_object;
            if (current_object != ECS_NULL_ENT)
            {
                struct InteractableComp* interactable = ecs_get(registry, current_object, InteractableComp);
                ui_menu_callback(interact_callback, registry);
                ui_menu_set_options(interactable->commnads);
                ui_menu_display();
            }
        }
    }
}

static void
sync_hand_direction(ecs_Registry* registry)
{
    struct ecs_View view = { 0 };

    struct RefComp*             ref_comp;
    struct FacingDirectionComp* facing_direction_comp;
    struct ControllerComp*      controller_comp;

    void*        components[3];
    ecs_entity_t ett;
    ecs_view_init(&view, registry, { RefComp, FacingDirectionComp, ControllerComp });
    while (ecs_view_next(&view, &ett, components))
    {
        ref_comp              = components[0];
        facing_direction_comp = components[1];
        controller_comp       = components[2];

        bool should_flip = facing_direction_comp->value[0] < 0.f;
        if (ref_comp->hand != ECS_NULL_ENT && !controller_comp->in_action)
        {
            float angle_in_rad = SDL_atan2f(facing_direction_comp->value[1], facing_direction_comp->value[0]);

            struct TransformComp* transform = ecs_get(registry, ref_comp->hand, TransformComp);

            transform->position[0] = 4 * (float)signf(facing_direction_comp->value[0]);
            transform->rotation    = glm_deg(angle_in_rad);

            if (transform->rotation < 0.f)
            {
                transform->rotation = 180.f + (transform->rotation + 180.f);
            }

            ecs_assure(registry, ref_comp->hand, TransformChangedTag);
        }
        if (ref_comp->weapon != ECS_NULL_ENT && !controller_comp->in_action)
        {
            ecs_get(registry, ref_comp->weapon, SpriteComp)->hori_flip = should_flip;
        }
    }
}

void
system_input_update(ecs_Registry* registry, OthoCamera* camera)
{
    sync_facing_direction(registry, camera);
    sync_hand_direction(registry);
    update_desired_direction(registry);
}
