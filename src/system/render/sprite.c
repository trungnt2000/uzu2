#include "components.h"
#include "graphics.h"
#include "graphics/view.h"
#include "system_render.h"

#define MAX_SPRITE_PER_BATCH 512

//typedef union  Dependencies {
//  void* comps[2];
//  struct {
//    _Sprite*          sprite;
//    _TransformMatrix* transformMatrix;
//  };
//} Dependencies;

typedef struct Dependencies
{
  _Sprite*          sprite;
  _TransformMatrix* transformMatrix;
} Dependencies;

static GLuint sShader;
static int    sViewProjectionMatrixLocation;

static ecs_Group sSpriteGroup;

void
system_rendering_sprite_init(ecs_Registry* registry)
{
  ecs_group_init(&sSpriteGroup, registry, { Sprite, TransformMatrix });
  sprite_renderer_init(MAX_SPRITE_PER_BATCH);
  if (create_shader("res/shader/sprite.vert",
                    "res/shader/sprite.frag",
                    &sShader) != 0)
  {
    UZU_ERROR("Faild to load shader...\n");
  }
  sViewProjectionMatrixLocation =
      glGetUniformLocation(sShader, "u_viewProjectionMatrix");
}

void
system_rendering_sprite_fini(void)
{
  sprite_renderer_shutdown();
  ecs_group_destroy(&sSpriteGroup);
  glDeleteProgram(sShader);
}

void
system_rendering_sprite_update(ecs_Registry* registry)
{
#if 0
  ecs_View     view;
  ecs_entity_t ett;
  Dependencies deps;

  ecs_view_init(&view, registry, { Sprite, TransformMatrix });

  mat4 viewProjectionMatrix;
  view_combined(viewProjectionMatrix);
  glUseProgram(sShader);
  glUniformMatrix4fv(sViewProjectionMatrixLocation,
                     1,
                     GL_FALSE,
                     viewProjectionMatrix[0]);
  sprite_batch_begin();
  glEnable(GL_DEPTH_TEST);
  while (ecs_view_next(&view, &ett, &deps))
  {

    draw_sprite(deps.sprite->size,
                deps.sprite->origin,
                deps.sprite->color,
                0,
                &deps.sprite->textureRegion,
                deps.transformMatrix->value);
  }
  glDisable(GL_DEPTH_TEST);
  sprite_batch_end();
#endif

#if 1
  _Sprite*            sp  = ecs_group_data_begin(&sSpriteGroup, 0);
  _TransformMatrix*   tx  = ecs_group_data_begin(&sSpriteGroup, 1);
  const ecs_entity_t* ett = ecs_group_ett_begin(&sSpriteGroup);
  int                 siz = ecs_group_size(&sSpriteGroup);

  mat4 viewProjectionMatrix;
  view_combined(viewProjectionMatrix);
  glUseProgram(sShader);
  glUniformMatrix4fv(sViewProjectionMatrixLocation,
                     1,
                     GL_FALSE,
                     viewProjectionMatrix[0]);

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
#endif
}
