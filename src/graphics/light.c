#include "graphics/light.h"
#include "graphics/gl.h"
#include "graphics/view.h"

#define U_LIGHT_POSITION_NAME "u_LightPosition"
#define U_LIGHT_COLOR_NAME "u_LightColor"
#define U_LIGHT_INTENSITY_NAME "u_LightInstensity"
#define U_LIGHT_RADIUS_NAME "u_LightRadius"
#define U_PROJECTION_MATRIX "u_ProjectionMatrix"
#define U_INVERT_VIEW_MATRIX "u_InvertViewMatrix"

static const char sLightShaderVertexSource[] =
    /**/
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_Position;"
    "layout (location = 1) in vec2 a_TexCoords;"
    "uniform vec2 u_LightPosition;"
    "uniform vec4 u_LightColor;"
    "uniform float u_LightInstensity;"
    "uniform float u_LightRadius;"
    "uniform mat4 u_ProjectionMatrix;"
    "uniform mat4 u_InvertViewMatrix;"
    "out vec4 LightColor;"
    "out float Intensity;"
    "out vec2 TexCoords;"
    "void main() {"
    "    vec2 realPosition = (u_InvertViewMatrix * vec4(a_Position, 0.f, 1.f)).xy;"
    "    float dist = max(distance(realPosition, u_LightPosition), 1.f);"
    "    LightColor = u_LightColor;"
    "    Intensity = (u_LightRadius / dist) * u_LightInstensity * 0.5;"
    "    TexCoords = a_TexCoords;"
    "    gl_Position = u_ProjectionMatrix * vec4(a_Position, 0.f, 1.f);"
    "}";

static const char sLightShaderFragmentSource[] =
    /**/
    "#version 330 core\n"
    "uniform sampler2D u_MainTex;"
    "in vec4 LightColor;"
    "in float Intensity;"
    "in vec2 TexCoords;"
    "out vec4 FragColor;"
    "void main() {"
    "    FragColor = texture(u_MainTex, TexCoords);"
    "}";

extern int create_shader_form_sources(const char* vsSource, const char* fsSource, unsigned int* shaderReturn);

int
light_shader_load(LightShader* shader)
{
  if (create_shader_form_sources(sLightShaderVertexSource, sLightShaderFragmentSource, &shader->handle) != 0)
  {
    UZU_ERROR("Failed to load light shader!\n");
    return -1;
  }
  shader->uLightPositionLoc    = glGetUniformLocation(shader->handle, U_LIGHT_POSITION_NAME);
  shader->uLightColorLoc       = glGetUniformLocation(shader->handle, U_LIGHT_COLOR_NAME);
  shader->uLightInstensityLoc  = glGetUniformLocation(shader->handle, U_LIGHT_INTENSITY_NAME);
  shader->uLightRadiusLoc      = glGetUniformLocation(shader->handle, U_LIGHT_RADIUS_NAME);
  shader->uInvertViewMatrixLoc = glGetUniformLocation(shader->handle, U_INVERT_VIEW_MATRIX);
  shader->uProjectionMatrixLoc = glGetUniformLocation(shader->handle, U_PROJECTION_MATRIX);
  return 0;
}

void
light_shader_destroy(LightShader* shader)
{
  glDeleteProgram(shader->handle);
  shader->handle = 0;
}

static void
prepare_light(const LightShader* shader, const Light* light)
{
  glUniform2fv(shader->uLightPositionLoc, 1, light->position);
  glUniform4fv(shader->uLightColorLoc, 1, light->color);
  glUniform1f(shader->uLightInstensityLoc, light->intensity);
  glUniform1f(shader->uLightRadiusLoc, light->radius);
}

static void
prepare_light_shader(const LightShader* shader, OthoCamera* view)
{
  mat4 projectionMatrix;
  mat4 invertViewMatrix;
  otho_camera_get_invert_view_matrix(view, invertViewMatrix);
  otho_camera_get_projection_matrix(view, projectionMatrix);

  glUseProgram(shader->handle);
  glUniformMatrix4fv(shader->uProjectionMatrixLoc, 1, GL_FALSE, (float*)projectionMatrix);
  glUniformMatrix4fv(shader->uInvertViewMatrixLoc, 1, GL_FALSE, (float*)invertViewMatrix);
}

static void
init_quad_vao(LightingPass* lightingPass, float width, float height)
{

  //clang-format off
  /*
  float vertiecs[] = {
    -1.f, -1.f, 0.f, 0.f, //0
    -1.f, 1.f, 0.f, 1.f, //1
    1.f, 1.f, 1.f, 1.f, //2
    1.f, 1.f, 1.f, 1.f, //2
    1.f, -1.f, 1.f, 0.f, //3
    -1.f, -1.f, 0.f, 0.f, //0
  };*/
  float vertiecs[] = {
    0.f, 0.f, 0.f, 0.f, //0
    width, 0.f, 1.f, 0.f, //1
    0.f, height, 0.f, 1.f, //3
    width, 0.f, 1.f, 0.f, //1
    width, height, 1.f, 1.f,//2
    0.f, height, 0.f, 1.f, //3
  };
  //clang-format on

  glGenVertexArrays(1, &lightingPass->vao);
  glBindVertexArray(lightingPass->vao);

  glGenBuffers(1, &lightingPass->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, lightingPass->vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof vertiecs, vertiecs, GL_STATIC_DRAW);

  size_t offset = 0;
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)offset);
  glEnableVertexAttribArray(0);
  offset += sizeof(float) * 2;

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)offset);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int
lighting_pass_init(LightingPass* lightingPass, int scrWidth, int scrHeight)
{
  init_quad_vao(lightingPass, (float)scrWidth, (float)scrHeight);
  framebuffer_init(&lightingPass->framebuffer, scrWidth, scrHeight);
  light_shader_load(&lightingPass->shader);
  return 0;
}

void
lighting_pass_destroy(LightingPass* lightingPass)
{
  glDeleteVertexArrays(1, &lightingPass->vao);
  lightingPass->vao = 0;

  glDeleteBuffers(1, &lightingPass->vbo);
  lightingPass->vbo = 0;

  framebuffer_destroy(&lightingPass->framebuffer);
  light_shader_destroy(&lightingPass->shader);
}

void
lighting_pass_render(LightingPass*  lightingPass,
                     OthoCamera*          view,
                     const Light*   lights,
                     int            count,
                     const Texture* texture /*, QuadWriter* quadWriter*/)
{
  /* prepare */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  texture_bind(texture);
  glActiveTexture(GL_TEXTURE0);
  prepare_light_shader(&lightingPass->shader, view);

  //framebuffer_bind(&lightingPass->framebuffer);
  //glViewport(0, 0, view->size[0], view->size[1]); 
  
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(lightingPass->vao);
  for (int i = 0; i < count; ++i)
  {
    prepare_light(&lightingPass->shader, &lights[i]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  glUseProgram(0);
  texture_unbind(texture);
  glBindVertexArray(0);
  //framebuffer_unbind(&lightingPass->framebuffer);

  glDisable(GL_BLEND);
}

const Texture*
lighting_pass_get_output(LightingPass* lightingPass)
{
  return &lightingPass->framebuffer.target;
}
