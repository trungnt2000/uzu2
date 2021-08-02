#include "components.h"
#include "graphics.h"
#include "system_render.h"
#include "graphics/view.h"

#define MAX_SPRITE_PER_BATCH 512

typedef struct Dependencies
{
  _Sprite*          sprite;
  _TransformMatrix* transformMatrix;
} Dependencies;

static SpriteRS* sRenderState;
static GLuint    sShader;
static int       sViewProjectionMatrixLocation;

void
system_rendering_sprite_init(void)
{
  sRenderState = sprite_render_state_create(MAX_SPRITE_PER_BATCH);
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
  sprite_render_state_destroy(sRenderState);
  glDeleteProgram(sShader);
}

void
system_rendering_sprite_update(ecs_Registry* registry)
{
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
  begin_draw_sprite(sRenderState);
  glEnable(GL_DEPTH_TEST);
  while (ecs_view_next(&view, &ett, &deps))
  {

    draw_sprite(sRenderState,
                deps.sprite->size,
                deps.sprite->origin,
                deps.sprite->color,
                0,
                &deps.sprite->textureRegion,
                deps.transformMatrix->value);
  }
  glDisable(GL_DEPTH_TEST);
  end_draw_sprite(sRenderState);
}
