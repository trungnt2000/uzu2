#include "graphics.h"

static char*
read_entrie_file(const char* file)
{
  SDL_RWops* rwops;
  char*      buff;

  if ((rwops = SDL_RWFromFile(file, "r")))
  {
    SDL_RWseek(rwops, 0, RW_SEEK_END);
    int length = SDL_RWtell(rwops);
    buff       = SDL_malloc(length + 1);

    SDL_RWseek(rwops, 0, RW_SEEK_SET);
    SDL_RWread(rwops, buff, 1, length);
    buff[length] = 0;
    return buff;
  }
  return NULL;
}

static BOOL
compile_file(const char* file, GLenum type, GLuint* out)
{
  char*  source;
  GLuint shader;
  int    isCompiled;
  int    infoLogLength;
  char*  infoLog;

  source = read_entrie_file(file);
  if (!source)
  {
    ERROR("Fail to read file: %s\n", file);
    return FALSE;
  }

  shader = glCreateShader(type);
  glShaderSource(shader, 1, (const char* const*)&source, NULL);

  glCompileShader(shader);

  SDL_free(source);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    infoLog = SDL_malloc(infoLogLength);
    glGetShaderInfoLog(shader, infoLogLength, &infoLogLength, infoLog);
    ERROR("Fail to compile_file shader: %s\n", infoLog);
    SDL_free(infoLog);
    return FALSE;
  }
  *out = shader;

  return TRUE;
}

int
create_shader(const char* vsFile, const char* fsFile, GLuint* outProgram)
{
  GLuint vertShader = 0, fragShader = 0;
  GLuint program = 0;
  char   linkingInfo[512];
  int    isLinkingSucceed;

  if (!vsFile || !fsFile || !outProgram)
  {
    ERROR("Invalid argument\n");
    return -1;
  }

  if (!compile_file(vsFile, GL_VERTEX_SHADER, &vertShader))
  {
    ERROR("Fail to compile vertex shader\n");
    return -1;
  }

  if (!compile_file(fsFile, GL_FRAGMENT_SHADER, &fragShader))
  {
    ERROR("Fail to compile fragment shader");
    glDeleteShader(vertShader);
    return -1;
  }

  program = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &isLinkingSucceed);
  if (isLinkingSucceed != GL_TRUE)
  {
    glGetProgramInfoLog(program, 512, NULL, linkingInfo);
    glDeleteProgram(program);
    ERROR("Fail to link program %s", linkingInfo);
    return -1;
  }

  *outProgram = program;
  return 0;
}

/***************************************************************/

int
sprite_shader_load(SpriteShader* shader)
{
  if (create_shader("res/shader/sprite.vert",
                    "res/shader/sprite.frag",
                    &shader->handle) != 0)
  {
    ERROR("Failed to create program\n");
    return -1;
  }
  shader->uProjMatLocation =
      glGetUniformLocation(shader->handle, "u_viewProjectionMatrix");
  return 0;
}

void
sprite_shader_destroy(SpriteShader* shader)
{
  glDeleteProgram(shader->handle);
  shader->handle           = 0;
  shader->uProjMatLocation = -1;
}

void
sprite_shader_uniform_projmat(SpriteShader* shader, mat4 projMat)
{
  glUniformMatrix4fv(shader->uProjMatLocation, 1, GL_FALSE, (float*)projMat);
}
