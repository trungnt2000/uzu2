#include "components.h"
#include "config.h"
#include "constances.h"
#include "ecs.h"
#include "engine.h"
#include "graphics/view.h"
#include "input.h"
#include "level_loader.h"
#include "map.h"
#include "map_render.h"
#include "system_render.h"

void scene_main_load_level(const char* level, const char* dstPortal);

static ecs_Registry* sRegistry;

static Texture      texture;
static Texture      lizzardTexture;
static ecs_entity_t entity1;
static ecs_entity_t entity2;
static Animation    lizzardAnim;

/*
static BOOL sHasPendingLevel;
static char sPendingLevel[LADDER_ATTRS_MAX_LEVEL_NAME_LEN + 1];
static char sTargetPortal[LADDER_ATTRS_MAX_DEST_LEN + 1];
static BOOL sIsPaused;
static BOOL sIsPlayerDied;

#if DEBUG
static BOOL sIsTileColliderDebugEnabled;
static BOOL sIsHitboxDebugEnabled;
static BOOL sIsPosDebugEnabled;
static BOOL sIsRtreeDebugEnabled;
static BOOL sIsGirdDebugEnabled;
static BOOL sIsPathDebugEnabled;
#endif
*/

static void preupdate(float deltaTime);
static void update(float deltaTime);
static void postupdate(float deltaTime);

static void
process_input(void* SDL_UNUSED arg, u32 currState, SDL_UNUSED u32 prevState)
{
  const float speed     = 64.f;
  const float deltaTime = engine_deltatime();

  if (currState & BTN_MSK_LEFT)
    view_translate((vec2){ speed * deltaTime, 0.f });
  if (currState & BTN_MSK_RIGHT)
    view_translate((vec2){ -speed * deltaTime, 0.f });
  if (currState & BTN_MSK_UP)
    view_translate((vec2){ 0, -speed * deltaTime });
  if (currState & BTN_MSK_DOWN)
    view_translate((vec2){ 0, speed * deltaTime });

  if (key_just_pressed(SDL_SCANCODE_K))
    view_zoom((vec2){ 1.1f, 1.1f });
  if (key_just_pressed(SDL_SCANCODE_J))
    view_zoom((vec2){ 0.9f, 0.9f });
  if (key_pressed(SDL_SCANCODE_L))
    view_rotate(1.f);
  if (key_pressed(SDL_SCANCODE_H))
    view_rotate(-1.f);
}

void
scene_main_create(void)
{
  load_level("gl_test");
  view_reset(0, 0, WIN_WIDTH, WIN_HEIGHT);

  if (texture_load(&texture, "res/test.png") != 0)
  {
    ERROR("Could not load test texture\n");
  }

  if (texture_load(&lizzardTexture, "res/job/lizzard.png") != 0)
  {
    ERROR("Could not load lizzard texture\n");
  }

  AnimationTemplate tmpl = { 0 };
  tmpl.columnCount       = 5;
  tmpl.rowCount          = 1;
  tmpl.spriteWidth       = 16;
  tmpl.spriteHeight      = 28;
  tmpl.texture           = &lizzardTexture;
  tmpl.frameDuration     = .1f;
  tmpl.xOffset           = 4 * 16;

  anim_init_tmpl(&lizzardAnim, &tmpl);

  map_render_init();
  system_rendering_sprite_init();
  sRegistry = ecs_registry_create(gCompTraits, COMPONENT_CNT);

  entity1 = ecs_create(sRegistry);

  ecs_set(sRegistry,
          entity1,
          Transform,
          {
              .scale    = { 1.f, 1.f },
              .position = { 100.f, -30.f },
              .rotation = 50.f,
          });

  _Sprite* s = ecs_set(sRegistry,
                       entity1,
                       Sprite,
                       {
                           .textureRegion = { 0 },
                           .color         = { 1.f, 1.f, 1.f, 1.f },
                           .size          = { 32.f, 48.f },
                           .origin        = { 16.f, 48.f },
                       });
  texture_region_set_texture(&s->textureRegion, &texture, NULL);

  ecs_set(sRegistry, entity1, TransformMatrix, { GLM_MAT3_IDENTITY_INIT });
  ecs_add(sRegistry, entity1, TransformChanged);

  ecs_set(sRegistry,
          entity1,
          AnimationPool,
          {
              .anims   = &lizzardAnim,
              .animCnt = 1,
          });

#if 1
  entity2 = ecs_create(sRegistry);

  ecs_set(sRegistry,
          entity2,
          Transform,
          {
              .scale    = { 1.5f, 1.5f },
              .position = { 0.f, 0.f },
              .rotation = 0.f,
          });

  ecs_set(sRegistry, entity2, TransformMatrix, { GLM_MAT3_IDENTITY_INIT });
  ecs_add(sRegistry, entity2, TransformChanged);

  s = ecs_set(sRegistry,
              entity2,
              Sprite,
              {
                  .textureRegion = { 0 },
                  .color         = { 1.f, 0.5f, 0.5f, 1.f },
                  .size          = { 70.f, 70.f },
                  .origin        = { 35.f, 35.f },
              });
  texture_region_set_texture(&s->textureRegion,
                             &texture,
                             &(IntRect){ .w = 288, .h = 300 });
#endif
}

void
scene_main_destroy(void)
{
  map_render_fini();
  printf("scene_main destroyed!\n");
  ecs_registry_free(sRegistry);
  system_rendering_sprite_fini();
  anim_destroy(&lizzardAnim);
  texture_free(&texture);
  texture_free(&lizzardTexture);
}

void
scene_main_tick(float deltaTime)
{
  map_tick();
  map_render();
  system_rendering_transform_update(sRegistry);
  system_rendering_animation_update(sRegistry, deltaTime);
  system_rendering_sprite_update(sRegistry);

  _Transform* tx = ecs_get(sRegistry, entity1, Transform);
  ecs_add(sRegistry, entity1, TransformChanged);

  int x, y;
  SDL_GetMouseState(&x, &y);
  tx->position[0] =
      lerpf(tx->position[0], (x) / SCL_X - WIN_WIDTH / 2, deltaTime * 5.f);
  tx->position[1] =
      lerpf(tx->position[1], (y) / SCL_Y - WIN_HEIGHT / 2, deltaTime * 5.f);
  tx->rotation += 1.f;
}

void
scene_main_enter(void)
{
  input_push(INPUT_CALLBACK1(process_input));
}

void
scene_main_leave(void)
{
  input_pop();
}

void
scene_main_receive_event(const SDL_Event* event)
{
}
