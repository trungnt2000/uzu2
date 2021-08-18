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
#include "system_logic.h"
#include "system_render.h"

void scene_main_load_level(const char* level, const char* dstPortal);

static ecs_Registry* sRegistry;

static Texture      texture;
static Texture      lizzardTexture;
static ecs_entity_t entity1;
static ecs_entity_t entity2;
static Animation    lizzardAnim;
static SpriteShader shader;
static Font         font;
static float        scl = 1.f;

static void preupdate(float deltaTime);
static void update(float deltaTime);
static void postupdate(float deltaTime);

static const char text[] = "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n"
                           "The quick brown fox jumps over the lazy dog.\n";

static const char text2[] =
    "&buffer is the address of the first character position where the input string will be stored. It’s not "
    "the base address of the buffer, but of the first character in the buffer. This pointer type (a "
    "pointer-pointer or the ** thing) causes massive confusion.\n"
    "&size is the address of the variable that holds the size of the input buffer, another pointer.\n"
    "stdin is the input file handle. So you could use getline() to read a line of text from a file, but when "
    "stdin is specified, standard input is read.\n";

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

  if (key_pressed(SDL_SCANCODE_V))
    scl+=0.1f;
  
  if (key_pressed(SDL_SCANCODE_B))
    scl-=0.1f;
}

void
scene_main_create(void)
{

  load_level("gl_test");
  view_reset(WIN_WIDTH / 2, WIN_HEIGHT / 2, WIN_WIDTH, WIN_HEIGHT);

  if (texture_load(&texture, "res/test.png") != 0)
  {
    UZU_ERROR("Could not load test texture\n");
  }

  if (texture_load(&lizzardTexture, "res/job/lizzard.png") != 0)
  {
    UZU_ERROR("Could not load lizzard texture\n");
  }

  if (sprite_shader_load(&shader) != 0)
  {
    UZU_ERROR("Could not load default shader\n");
  }

  if (font_load(&font, "res/font/font.TTF", 16) != 0)
  {
    UZU_ERROR("Could not load font!\n");
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

  // map_renderer_init();
  sRegistry = ecs_registry_create(gCompTraits, COMPONENT_CNT);
  system_rendering_sprite_init(sRegistry);

  entity1 = ecs_create(sRegistry);

  ecs_add_ex(sRegistry,
             entity1,
             Transform,
             {
                 .scale    = { 1.f, 1.f },
                 .position = { 100.f, -30.f },
                 .rotation = 50.f,
             });

  _Sprite* s = ecs_add_ex(sRegistry,
                          entity1,
                          Sprite,
                          {
                              .textureRegion = { 0 },
                              .color         = { 1.f, 1.f, 1.f, 1.f },
                              .size          = { 32.f, 48.f },
                              .origin        = { 16.f, 48.f },
                          });
  texture_region_set_texture(&s->textureRegion, &texture, NULL);

  ecs_add_ex(sRegistry, entity1, TransformMatrix, { GLM_MAT3_IDENTITY_INIT });
  ecs_add(sRegistry, entity1, TransformChanged);

  ecs_add_ex(sRegistry,
             entity1,
             AnimationPool,
             {
                 .anims   = &lizzardAnim,
                 .animCnt = 1,
             });

  int w = 50 * 16;
  int h = 50 * 16;
  for (int i = 0; i < 10; ++i)
  {
    ecs_entity_t ett = ecs_create(sRegistry);

    ecs_add_ex(sRegistry,
               ett,
               Transform,
               {
                   .scale    = { 1.f, 1.f },
                   .position = { 0.f, i * 10.f },
                   .rotation = rand() % 360,
               });

    s = ecs_add_ex(sRegistry,
                   ett,
                   Sprite,
                   {
                       .textureRegion = { 0 },
                       .color         = { 1.f, 1.f, 1.f, 1.f },
                       .size          = { 32.f, 48.f },
                       .origin        = { 16.f, 48.f },
                   });
    texture_region_set_texture(&s->textureRegion, &texture, NULL);

    ecs_add_ex(sRegistry, ett, TransformMatrix, { GLM_MAT3_IDENTITY_INIT });
    ecs_add(sRegistry, ett, TransformChanged);

    ecs_add_ex(sRegistry,
               ett,
               AnimationPool,
               {
                   .anims   = &lizzardAnim,
                   .animCnt = 1,
               });

    if (i % 3 == 0)
    {
      ecs_add_ex(sRegistry, ett, Velocity, { .value = { 10.f, 0.f } });
    }
  }
}

void
scene_main_destroy(void)
{
  // map_renderer_fini();
  printf("scene_main destroyed!\n");
  ecs_registry_free(sRegistry);
  system_rendering_sprite_fini();
  anim_destroy(&lizzardAnim);
  texture_destroy(&texture);
  texture_destroy(&lizzardTexture);
}

void
scene_main_tick(float deltaTime)
{
#if 1
  // map_tick();
  // map_render();

  mat4 projectionViewMatrix;
  view_combined(projectionViewMatrix);
  sprite_shader_bind(&shader);
  sprite_shader_uniform_projmat(&shader, projectionViewMatrix);
  u32 start = SDL_GetTicks();
  system_motion_update(sRegistry, deltaTime);

  sprite_batch_begin();
  system_rendering_transform_update(sRegistry);
  system_rendering_animation_update(sRegistry, deltaTime);
  system_rendering_sprite_update(sRegistry);

  _Transform* tx = ecs_get(sRegistry, entity1, Transform);
  ecs_add(sRegistry, entity1, TransformChanged);

  int x, y;
  SDL_GetMouseState(&x, &y);

  float mx        = ((float)x / (float)SCL_X) + view_left();
  float my        = ((float)y / (float)SCL_Y) + view_top();
  tx->position[0] = lerpf(tx->position[0], mx, deltaTime * 5.f);
  tx->position[1] = lerpf(tx->position[1], my, deltaTime * 5.f);
  tx->rotation += 1.f;
#endif

  // draw_text(text, 50, 50, (vec4){ 0.f, 0.2f, 0.7f, 1.f });
  draw_text_boxed_ex(text2,
                     &font,
                     (vec2){ 0.f, 0.f },
                     (vec2){ 200, 200 },
                     scl,
                     TEXT_ALIGN_LEFT,
                     TEXT_WRAP_NORMAL,
                     COLOR_RED);
  sprite_batch_end();

  RenderStatistics statistics;
  char             buf[255];
  float            time = (float)(SDL_GetTicks() - start) / 1000.f;
  sprite_renderer_query_statistics(&statistics);
  sprintf(buf,
          "time: %.3fs\ndraw call: %u\nvertice count: %u",
          time,
          statistics.drawCall,
          statistics.verticeCount);
  mat4 projectionMatrix;
  view_projection_matrix(projectionMatrix);
  sprite_shader_uniform_projmat(&shader, projectionMatrix);
  sprite_batch_begin();
  draw_textv_ex(buf, &font, (vec2){ 0.f, 0.f }, 1.f, TEXT_ALIGN_LEFT, COLOR_GREEN);
  sprite_batch_end();
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
