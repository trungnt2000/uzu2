#include "components.h"
#include "graphics.h"
#include "system_render.h"

static ecs_Group* s_sprite_group;

void
system_rendering_sprite_init(struct ecs_Registry* registry)
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
    struct SpriteComp*               sp  = ecs_group_data_begin(s_sprite_group, 0);
    struct WorldTransformMatrixComp* tx  = ecs_group_data_begin(s_sprite_group, 1);
    struct MaterialComp*             mt  = ecs_group_data_begin(s_sprite_group, 2);
    int                              siz = ecs_group_size(s_sprite_group);

    for (int i = 0; i < siz; ++i)
    {
        draw_sprite_ex(&sp[i].sprite,
                       mt[i].ref,
                       tx[i].value,
                       sp[i].origin,
                       sp[i].color,
                       sp[i].vert_flip,
                       sp[i].hori_flip);
    }
}
