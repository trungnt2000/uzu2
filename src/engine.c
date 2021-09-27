#include "engine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "config.h"
#include "constances.h"
#include "graphics.h"
#include "graphics/gl.h"
#include "input.h"
#include "toolbox.h"

#include "gameconf.h"

#include "SDL_opengl.h"

#ifndef MAX_SPRITE_PER_BATCH
#define MAX_SPRITE_PER_BATCH 2048
#endif

static bool          s_is_running = false;
static SDL_GLContext s_gl_context;
static float         s_delta_time;
static SDL_Window*   s_window;

extern void configure(GameConf* conf);
extern bool create(void);
extern void receive_event(const SDL_Event* event);
extern void tick(float delta_time);
extern void destroy(void);

#if DEBUG
static void GLAPIENTRY
message_callback(UNUSED GLenum      source,
                 GLenum             type,
                 UNUSED GLuint      id,
                 UNUSED GLenum      severity,
                 UNUSED GLsizei     length,
                 const GLchar*      message,
                 UNUSED const void* user_param)
{
    if (type != GL_DEBUG_TYPE_ERROR)
        return;

    UZU_ERROR("%s\n", message);
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

    GameConf conf = { .width = 620, .height = 480, .vsync = true };
    configure(&conf);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    s_window = SDL_CreateWindow(WIN_TITLE,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                conf.width,
                                conf.height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    s_gl_context = SDL_GL_CreateContext(s_window);

    if (s_gl_context == NULL)
    {
        UZU_ERROR("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(conf.vsync ? 1 : 0);

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

    if (s_window == NULL)
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

    text_renderer_init(MAX_SPRITE_PER_BATCH);
    sprite_renderer_init(MAX_SPRITE_PER_BATCH);

    if (!create())
        return false;

    return true;
}

static int
run()
{
    SDL_Event event;
    Uint32    last_time, current_time;
    last_time    = SDL_GetTicks();
    s_is_running = SDL_TRUE;

    while (s_is_running)
    {
        current_time = SDL_GetTicks();
        s_delta_time = (float)(current_time - last_time) / 1000.f;
        last_time    = current_time;

        while (SDL_PollEvent(&event))
        {
            receive_event(&event);
        }
        input_update();
        tick(s_delta_time);
        SDL_GL_SwapWindow(s_window);
    }

    return 0;
}

static void
cleanup(void)
{
    destroy();
    text_renderer_shutdown();
    font_loader_shutdown();
    SDL_GL_DeleteContext(s_gl_context);
    SDL_DestroyWindow(s_window);
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

void
engine_stop()
{
    s_is_running = false;
}

float
engine_deltatime()
{
    return s_delta_time;
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
