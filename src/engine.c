#include "engine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "config.h"
#include "constances.h"
#include "graphics.h"
#include "input.h"
#include "toolbox.h"

#include "graphics/gl.h"
#include "SDL_opengl.h"

#ifndef MAX_SPRITE_PER_BATCH
#define MAX_SPRITE_PER_BATCH 2048
#endif

static bool          sIsRunning = false;
static SDL_GLContext sGLCtx;
static float         sDeltaTime;

static SDL_Window* sWindow;

extern void tick(float deltaTime);
extern bool create(void);
extern void destroy(void);
extern void receive_event(const SDL_Event* event);

#if DEBUG
static void GLAPIENTRY
message_callback(SDL_UNUSED GLenum      source,
                 GLenum                 type,
                 SDL_UNUSED GLuint      id,
                 SDL_UNUSED GLenum      severity,
                 SDL_UNUSED GLsizei     length,
                 const GLchar*          message,
                 SDL_UNUSED const void* userParam)
{
  if (type != GL_DEBUG_TYPE_ERROR)
    return;

  printf("[GLERROR] :  %s\n", message);
}
#endif

static bool
init()
{
  u32 flags = SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK |
              SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER;

  if (SDL_Init(flags) != 0)
  {
    UZU_ERROR("SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  sWindow = SDL_CreateWindow(WIN_TITLE,
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             (int)(WIN_WIDTH * SCL_X),
                             (int)(WIN_HEIGHT * SCL_Y),
                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  sGLCtx = SDL_GL_CreateContext(sWindow);

  if (sGLCtx == NULL)
  {
    UZU_ERROR("SDL_Init failed: %s\n", SDL_GetError());
    return UZU_FALSE;
  }

#ifdef _WIN32
  if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
  {
    UZU_ERROR("gladLoadGL failed\n");
    return UZU_FALSE;
  }
#endif // _WIN32

#if DEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(message_callback, 0);
#endif

  if (sWindow == NULL)
    return false;

  // TODO: replace with stb_image
  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    UZU_ERROR("failed to init SDL_image\n");
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
  {
    UZU_ERROR("open audio failed\n");
    return false;
  }

  input_init();

  if (font_loader_init() != 0)
  {
    UZU_ERROR("Could not init font loader\n");
    return false;
  }

  sprite_renderer_init(MAX_SPRITE_PER_BATCH);

  if (!create())
  {
    return false;
  }

  return true;
}

static int
run()
{
  SDL_Event event;
  Uint32    lastTime, currentTime;
  lastTime   = SDL_GetTicks();
  sIsRunning = SDL_TRUE;

  while (sIsRunning)
  {
    currentTime = SDL_GetTicks();
    sDeltaTime  = (float)(currentTime - lastTime) / 1000.f;
    lastTime = currentTime;

    while (SDL_PollEvent(&event))
    {
      receive_event(&event);
    }
    input_update();
    tick(sDeltaTime);
    SDL_GL_SwapWindow(sWindow);
  }

  return 0;
}

static void
cleanup(void)
{
  destroy();
  sprite_renderer_shutdown();
  font_loader_shutdown();
  SDL_GL_DeleteContext(sGLCtx);
  SDL_DestroyWindow(sWindow);
  IMG_Quit();
  Mix_Quit();
  SDL_Quit();
}

void
engine_stop()
{
  sIsRunning = false;
}

float
engine_deltatime()
{
  return sDeltaTime;
}

int
main(int argc, char** argv)
{
  if (init())
  {
    run();
  }
  cleanup();
  return EXIT_SUCCESS;
}
