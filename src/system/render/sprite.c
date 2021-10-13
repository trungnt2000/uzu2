#include "components.h"
#include "graphics.h"
#include "system_render.h"

static ecs_Group* s_sprite_group;

enum SortingLayer
{
    SORTING_LAYER_SPRITE
};

void
system_rendering_sprite_init(ecs_Registry* registry)
{
    ecs_create_group(registry, s_sprite_group, { SpriteComp, WorldTransformMatrixComp, MaterialComp });
}

void
system_rendering_sprite_shutdown(void)
{
    s_sprite_group = NULL;
}

void
system_rendering_sprite_update(SDL_UNUSED ecs_Registry* registry)
{
    struct SpriteComp*               sp    = ecs_group_data_begin(s_sprite_group, 0);
    struct WorldTransformMatrixComp* tx    = ecs_group_data_begin(s_sprite_group, 1);
    const struct MaterialComp*       mt    = ecs_group_data_begin(s_sprite_group, 2);
    ecs_size_t                       count = ecs_group_size(s_sprite_group);

    for (ecs_size_t i = 0; i < count; ++i)
    {
        vec3 world_position = { 0 };
        glm_mat3_mulv(tx[i].value, (vec3){ 0.f, 0.f, 1.f }, world_position);

        int draw_order = (int)(world_position[1] * 10.f);
        draw_sprite_ex(&sp[i].sprite,
                       mt[i].ref,
                       tx[i].value,
                       sp[i].origin,
                       sp[i].color,
                       sp[i].vert_flip,
                       sp[i].hori_flip,
                       SORTING_LAYER_SPRITE,
                       draw_order);
    }
}
