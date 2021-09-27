#include "graphics.h"
#include "graphics/gl.h"

static char*
read_entrie_file(const char* file)
{
  SDL_RWops* stream;
  char*      buff;

  if ((stream = SDL_RWFromFile(file, "r")) != NULL)
  {
    SDL_RWseek(stream, 0, RW_SEEK_END);
    s64 length = SDL_RWtell(stream);
    if (length == -1)
    {
      goto fail;
    }
    buff = SDL_malloc((size_t)length + 1);

    SDL_RWseek(stream, 0, RW_SEEK_SET);
    SDL_RWread(stream, buff, 1, (size_t)length);
    buff[length] = '\0';

    SDL_RWclose(stream);
    return buff;
  fail:
    SDL_RWclose(stream);
    return NULL;
  }
  return NULL;
}

static bool
compile_source(const char* source, GLenum type, GLuint* shaderReturn)
{

  GLuint shader;
  int    isCompiled;
  char   info[512];

  shader = glCreateShader(type);
  glShaderSource(shader, 1, (const char* const*)&source, NULL);

  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    glGetShaderInfoLog(shader, sizeof(info) - 1, NULL, info);
    UZU_ERROR("Fail to compile shader: %s\n", info);
    glDeleteShader(shader);
    return false;
  }
  *shaderReturn = shader;

  return true;
}

int
create_shader_form_file(const char* vsFile, const char* fsFile, unsigned int* outProgram)
{
  char* vsSrc  = NULL;
  char* fsSrc  = NULL;
  int   status = 0;

  if (!vsFile || !fsFile || !outProgram)
  {
    UZU_ERROR("Invalid argument\n");
    return -1;
  }

  vsSrc = read_entrie_file(vsFile);
  fsSrc = read_entrie_file(fsFile);

  if (!vsSrc || !fsFile)
  {
    UZU_ERROR("Failed to load shader source files!\n");
    status = -1;
    goto end;
  }

  if (create_shader_form_sources(vsSrc, fsSrc, outProgram) != 0)
  {
    status = -1;
    goto end;
  }

end:
  SDL_free(vsSrc);
  SDL_free(fsSrc);
  return status;
}

int
create_shader_form_sources(const char* vsSrc, const char* fsSrc, unsigned int* outProgram)
{
  GLuint vertShader = 0;
  GLuint fragShader = 0;
  GLuint program    = 0;
  char   info[512];
  int    succeed;

  if (!vsSrc || !fsSrc || !outProgram)
  {
    UZU_ERROR("Invalid argument\n");
    return -1;
  }

  if (!compile_source(vsSrc, GL_VERTEX_SHADER, &vertShader))
  {
    UZU_ERROR("Fail to compile vertex shader\n");
    goto fail;
  }

  if (!compile_source(fsSrc, GL_FRAGMENT_SHADER, &fragShader))
  {
    UZU_ERROR("Fail to compile fragment shader\n");
    goto fail;
  }

  program = glCreateProgram();

  /* link shader prog */
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);

  /* make sure that we linked successfully */
  glGetProgramiv(program, GL_LINK_STATUS, &succeed);
  if (!succeed)
  {
    glGetProgramInfoLog(program, sizeof(info) - 1, NULL, info);
    UZU_ERROR("Fail to link program %s", info);
    goto fail;
  }

  /* after linking we no longer need those objects */
  glDeleteShader(fragShader);
  glDeleteShader(vertShader);
  *outProgram = program;
  return 0;

fail:
  glDeleteShader(fragShader);
  glDeleteShader(vertShader);
  glDeleteProgram(program);
  *outProgram = 0;
  return -1;
}

int
shader_load(Shader* shader, const char* vsFile, const char* fsFile)
{
  char* vsSrc  = read_entrie_file(vsFile);
  char* fsSrc  = read_entrie_file(fsFile);
  int   status = 0;

  if (vsSrc == NULL || fsSrc == NULL)
  {
    UZU_ERROR("Failed to load vsFile or fsFile\n");
    status = -1;
    goto cleanup;
  }

  if (shader_load_from_source(shader, vsSrc, fsSrc) != 0)
  {
    status = -1;
    goto cleanup;
  }
cleanup:
  SDL_free(vsSrc);
  SDL_free(fsSrc);
  return status;
}

int
shader_load_from_source(Shader* shader, const char* vsSrc, const char* fsSrc)
{
  if (create_shader_form_sources(vsSrc, fsSrc, &shader->handle) != 0)
  {
    return -1;
  }

  shader->view_proj_matrix_loc = glGetUniformLocation(shader->handle, "u_viewProjectionMatrix");
  return 0;
}

void
shader_destroy(Shader* shader)
{
  glDeleteProgram(shader->handle);
  shader->handle           = 0;
  shader->view_proj_matrix_loc = -1;
}

void
shader_upload_view_project_matrix(const Shader* shader, mat4 projMat)
{
  glUniformMatrix4fv(shader->view_proj_matrix_loc, 1, GL_FALSE, (float*)projMat);
}

void
shader_upload_mat4(const Shader* shader, const char* name, mat4 mat)
{
    int location = glGetUniformLocation(shader->handle, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)mat);
}

void
shader_bind(const Shader* shader)
{
  glUseProgram(shader->handle);
}

void
shader_unbind(const Shader* shader)
{
  (void)shader;
  glUseProgram(0);
}
