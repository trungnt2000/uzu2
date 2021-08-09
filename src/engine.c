#include "engine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "config.h"
#include "constances.h"
#include "font_loader.h"
#include "graphics.h"
#include "graphics/gl.h"
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
                             512,
                             512,
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

  lastTime   = SDL_GetTicks();
  sIsRunning = SDL_TRUE;
  Texture texture;
  TextShader shader;
  if (create_shader("res/shader/text.vert",
                    "res/shader/text.frag",
                    &shader.handle) != 0)
  {
    UZU_ERROR("Failed to create program\n");
    return -1;
  }

  FontFace face = FontRoboto;
  if (font_loader_init())
  {
    UZU_ERROR("Could not init font loader\n");
    return -1;
  }
  if (font_loader_load(face, 75, 75))
  {
    UZU_ERROR("Could not load font FontPaletteMosaic\n");
    return -1;
  }

  //GLfloat vertices[] = {
  //  -0.99f, -0.99f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.5f, // Lower left corner
  //  -0.99f, 0.99f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // Upper left corner
  //  0.99f,  0.99f,  0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, // Upper right corner
  //  0.99f,  -0.99f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, // Lower right corner
  //};
  //GLuint           indices[] = { 0, 2, 1, 0, 2, 3 };
  const FontAtlas* atlas     = font_loader_get_atlas(face, 75 - 75);

  const char* string             = "Who's gamers";
  const int   length             = (int)strlen(string);
  const int   indicesPerRec      = 6;
  const int   elemsPerVert       = 3 + 3 + 2;
  const int   vertPerRec         = 4;
  const int   indicesCount       = indicesPerRec * length;
  const int   verticesFloatCount = (size_t)length * vertPerRec * elemsPerVert;
  GLfloat*    vertices2 = SDL_malloc(sizeof(GLfloat) * verticesFloatCount);
  GLuint*     indices2  = SDL_malloc(sizeof(GLuint) * indicesCount);

  const char* iter  = string;
  vec2        pos   = { -1.f, 0.f };
  float         scale = 0.004f;
  for (int i = 0; i < length; ++i)
  {
    int             elemsOffset = i * elemsPerVert * vertPerRec;
    const CharInfo* charInfo    = &atlas->charInfo[*(iter + i)];
    GLfloat*        vert1       = vertices2 + elemsOffset;
    GLfloat*        vert2       = vert1 + ((size_t)elemsPerVert * 1);
    GLfloat*        vert3       = vert1 + ((size_t)elemsPerVert * 2);
    GLfloat*        vert4       = vert1 + ((size_t)elemsPerVert * 3);

    float xW = pos[0] + charInfo->bitmapBearing[0] * scale;
    float yW =
        pos[1] - (charInfo->bitmapSize[1] - charInfo->bitmapBearing[1]) * scale;
    float w  = charInfo->bitmapSize[0] * scale;
    float h  = charInfo->bitmapSize[1] * scale;
    vert1[0] = xW;
    vert1[1] = yW;
    vert1[2] = 0.f;
    vert1[3] = 1.f; //
    vert1[4] = 1.f;
    vert1[5] = 1.f;
    vert1[6] = charInfo->texTopLeft[0];
    vert1[7] = charInfo->texBottomRight[1];

    vert2[0] = xW;
    vert2[1] = h + yW;
    vert2[2] = 0.f;
    vert2[3] = 1.f;
    vert2[4] = 1.f;
    vert2[5] = 1.f;
    vert2[6] = charInfo->texTopLeft[0];
    vert2[7] = charInfo->texTopLeft[1];

    vert3[0] = xW + w;
    vert3[1] = h + yW;
    vert3[2] = 0.f;
    vert3[3] = 1.f;
    vert3[4] = 1.f;
    vert3[5] = 1.f;
    vert3[6] = charInfo->texBottomRight[0];
    vert3[7] = charInfo->texTopLeft[1];

    vert4[0] = xW + w;
    vert4[1] = yW;
    vert4[2] = 0.f;
    vert4[3] = 1.f; //
    vert4[4] = 1.f;
    vert4[5] = 1.f;
    vert4[6] = charInfo->texBottomRight[0];
    vert4[7] = charInfo->texBottomRight[1];
    pos[0] += charInfo->advance[0] * scale;

    int     recOffset  = i * indicesPerRec;
    int     vertOffset = i * vertPerRec;
    GLuint* idxRec     = indices2 + recOffset;
    idxRec[0]          = 0 + vertOffset;
    idxRec[1]          = 2 + vertOffset;
    idxRec[2]          = 1 + vertOffset;
    idxRec[3]          = 0 + vertOffset;
    idxRec[4]          = 2 + vertOffset;
    idxRec[5]          = 3 + vertOffset;
  }

  // FILE*            f     = fopen("test.raw", "w");
  // fwrite(atlas->texture, 1, (size_t)atlas->width * (size_t)atlas->height, f);
  // fclose(f);
  glGenTextures(1, &texture.handle);
  glBindTexture(GL_TEXTURE_2D, texture.handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RED,
               atlas->width,
               atlas->height,
               0,
               GL_RED,
               GL_UNSIGNED_BYTE,
               atlas->texture);

  texture.width  = atlas->width;
  texture.height = atlas->height;
  glBindTexture(GL_TEXTURE_2D, 0);

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(GLfloat) * verticesFloatCount,
               vertices2,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(GLuint) * indicesCount,
               indices2,
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glVertexAttribPointer(1,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(float),
                        (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // GLuint tex;
  // glGenTextures(1, &tex);
  // glBindTexture(GL_TEXTURE_2D, tex);
  // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // glTexImage2D(GL_TEXTURE_2D,
  //             0,
  //             GL_RGBA,
  //             atlas->width,
  //             atlas->glyphMaxHeight,
  //             0,
  //             GL_RGBA,
  //             GL_UNSIGNED_BYTE,
  //             atlas->texture);
  glBindTexture(GL_TEXTURE_2D, texture.handle);
  GLuint tex0Uni = glGetUniformLocation(shader.handle, "tex0");
  glUseProgram(shader.handle);
  glUniform1i(tex0Uni, 0);

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
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

    // glUniform3f(glGetUniformLocation(shader.handle, "textColor"),
    //            1.f,
    //            1.f,
    //            0.5f);
    // render_text(atlas, VBO, "1AV45", 0.5, 0.5, 1.f, 1.f);
    // tick(sDeltaTime);
    SDL_GL_SwapWindow(sWindow);
  }
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteTextures(1, &texture.handle);
  glDeleteProgram(shader.handle);
  font_loader_destroy();

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
