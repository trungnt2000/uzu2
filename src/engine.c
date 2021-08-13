#include "engine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "config.h"
#include "constances.h"
#include "font_loader.h"
#include "graphics.h"
#include "graphics/gl.h"
#include "graphics/text.h"
#include "graphics/view.h"
#include "input.h"
#include "system_render.h"
#include "toolbox.h"

static BOOL          sIsRunning = UZU_FALSE;
static SDL_GLContext sGLCtx;
static float         sDeltaTime;

static SDL_Window* sWindow;

extern void tick(float deltaTime);
extern BOOL create(void);
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

static BOOL
init()
{
  u32 flags = SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
              SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
              SDL_INIT_GAMECONTROLLER /*| SDL_INIT_SENSOR*/;

  if (SDL_Init(flags) != 0)
  {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    SDL_Log("Failed to Init SDL");
    return UZU_FALSE;
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
                             800,
                             600,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                                 SDL_WINDOW_RESIZABLE);

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
    return UZU_FALSE;

  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    UZU_ERROR("failed to init SDL_image\n");
    return UZU_FALSE;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
  {
    UZU_ERROR("open audio failed\n");
    return UZU_FALSE;
  }
  input_init();

  if (!create())
  {
    return UZU_FALSE;
  }
  return UZU_TRUE;
}

static int
engine_run()
{
  SDL_Event event;
  Uint32    lastTime, currentTime;
  int       windowClientWidth, windowClientHeigth;
  SDL_GetWindowSize(sWindow, &windowClientWidth, &windowClientHeigth);
  lastTime   = SDL_GetTicks();
  sIsRunning = SDL_TRUE;
  Texture    texture;
  TextShader shader;
  if (create_shader("res/shader/text.vert",
                    "res/shader/text.frag",
                    &shader.handle) != 0)
  {
    UZU_ERROR("Failed to create program\n");
    return -1;
  }

  if (font_loader_init())
  {
    UZU_ERROR("Could not init font loader\n");
    return -1;
  }

  const char* fontDir = "res/font/font.ttf";
  FontAtlas atlas;
  if (font_atlas_load(&atlas, fontDir, 12))
  {
    UZU_ERROR("Could not generate atlas for \"%s\"\n", fontDir);
    return -1;
  }

  // GLfloat vertices[] = {
  //  -0.99f, -0.99f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.5f, // Lower left corner
  //  -0.99f, 0.99f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // Upper left corner
  //  0.99f,  0.99f,  0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, // Upper right corner
  //  0.99f,  -0.99f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, // Lower right corner
  //};
  // GLuint           indices[] = { 0, 2, 1, 0, 2, 3 };

  char      string1[31]   = "The quick brown fox jumps over";
  int       length        = (int)strlen(string1);
  const int indicesPerRec = 6;
  const int vertPerRec    = 4;

  int indicesCount  = length * indicesPerRec;
  int verticesCount = length * vertPerRec;

  TextVertex* vertices2 = SDL_malloc(sizeof(TextVertex) * verticesCount);
  GLuint*     indices2  = SDL_malloc(sizeof(GLuint) * indicesCount);

  vec2  pos   = { -1.f, 0.f };
  float scale = 1.0f;

  glGenTextures(1, &texture.handle);
  glBindTexture(GL_TEXTURE_2D, texture.handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RED,
               atlas.width,
               atlas.height,
               0,
               GL_RED,
               GL_UNSIGNED_BYTE,
               atlas.texture);

  texture.width  = atlas.width;
  texture.height = atlas.height;
  glBindTexture(GL_TEXTURE_2D, 0);

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(TextVertex) * verticesCount,
               NULL,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(GLuint) * indicesCount,
               NULL,
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, position));
  glVertexAttribPointer(1,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, color));
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, uv));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLuint tex0Uni = glGetUniformLocation(shader.handle, "tex0");
  glUseProgram(shader.handle);
  glBindTexture(GL_TEXTURE_2D, texture.handle);
  glUniform1i(tex0Uni, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  while (sIsRunning)
  {
    currentTime = SDL_GetTicks();
    sDeltaTime  = (float)(currentTime - lastTime) / 1000.f;
    lastTime    = currentTime;
    while (SDL_PollEvent(&event))
    {
      receive_event(&event);
    }
    input_update();

    text_indices_update(length, indices2);

    text_line_update(string1,
                     length,
                     &atlas,
                     vertices2,
                     vertPerRec,
                     scale,
                     windowClientWidth,
                     windowClientHeigth,
                     pos);

    glClearColor(0.2f, 0.5f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    tick(sDeltaTime);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    sizeof(TextVertex) * verticesCount,
                    vertices2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                    0,
                    sizeof(GLuint) * indicesCount,
                    indices2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(shader.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(sWindow);
  }
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteTextures(1, &texture.handle);
  glDeleteProgram(shader.handle);
  font_atlas_destroy(&atlas);
  font_loader_shutdown();
  return 0;
}

static void
cleanup(void)
{
  destroy();
  SDL_GL_DeleteContext(sGLCtx);
  SDL_DestroyWindow(sWindow);
  IMG_Quit();
  Mix_Quit();
  SDL_Quit();
}

void
engine_stop()
{
  sIsRunning = UZU_FALSE;
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
    engine_run();
  }
  cleanup();
  return EXIT_SUCCESS;
}
