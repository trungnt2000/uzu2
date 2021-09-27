#include "input.h"
#include "components.h"
#include "ecs.h"

static void
get_mouse_position(OthoCamera* view, vec2 position_return)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    otho_camera_to_view_coords(view, (vec2){ (float)(mx), (float)(my) }, position_return);
}

void
update_facing_direction(ecs_Registry* registry, OthoCamera* camera)
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

        glm_vec2_sub(mouse_position, transform->position, to_mouse);
        glm_vec2_normalize_to(to_mouse, facing_direction->value);
    }
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
    }
}

void
system_input_update(ecs_Registry* registry, OthoCamera* camera)
{
    update_facing_direction(registry, camera);
    update_desired_direction(registry);
}
