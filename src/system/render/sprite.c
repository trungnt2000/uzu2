#include "components.h"
#include "graphics.h"
#include "graphics/view.h"
#include "system_render.h"

#define MAX_SPRITE_PER_BATCH 512

static ecs_Group    sSpriteGroup;
static SpriteShader sDefaultShader;

void
system_rendering_sprite_init(ecs_Registry* registry)
{
  ecs_group_init(&sSpriteGroup, registry, { Sprite, TransformMatrix });
  if (sprite_shader_load(&sDefaultShader) != 0)
  {
    UZU_ERROR("Faild to load shader...\n");
  }
}

void
system_rendering_sprite_fini(void)
{
  sprite_shader_destroy(&sDefaultShader);
  ecs_group_destroy(&sSpriteGroup);
}

void
system_rendering_sprite_update(ecs_Registry* registry)
{
  _Sprite*          sp  = ecs_group_data_begin(&sSpriteGroup, 0);
  _TransformMatrix* tx  = ecs_group_data_begin(&sSpriteGroup, 1);
  int               siz = ecs_group_size(&sSpriteGroup);

  for (int i = 0; i < siz; ++i)
  {
    draw_texture_region_w_tx(&sp[i].textureRegion, sp[i].size, sp[i].origin, sp[i].color, 0, tx[i].value);
  }
}
