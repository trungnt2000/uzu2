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
  sprite_renderer_init(MAX_SPRITE_PER_BATCH);
  if (sprite_shader_load(&sDefaultShader) != 0)
  {
    UZU_ERROR("Faild to load shader...\n");
  }
}

void
system_rendering_sprite_fini(void)
{
  sprite_renderer_shutdown();
  sprite_shader_destroy(&sDefaultShader);
  ecs_group_destroy(&sSpriteGroup);
}

void
system_rendering_sprite_update(ecs_Registry* registry)
{
  _Sprite*          sp  = ecs_group_data_begin(&sSpriteGroup, 0);
  _TransformMatrix* tx  = ecs_group_data_begin(&sSpriteGroup, 1);
  int               siz = ecs_group_size(&sSpriteGroup);

  mat4 viewProjectionMatrix;
  view_combined(viewProjectionMatrix);
  sprite_shader_bind(&sDefaultShader);
  sprite_shader_uniform_projmat(&sDefaultShader, viewProjectionMatrix);

  sprite_batch_begin();
  for (int i = 0; i < siz; ++i)
  {
    draw_sprite(sp[i].size,
                sp[i].origin,
                sp[i].color,
                0,
                &sp[i].textureRegion,
                tx[i].value);
  }
  sprite_batch_end();
  sprite_shader_unbind(&sDefaultShader);
}
