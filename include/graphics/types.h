// ngotrung Tue 17 Aug 2021 08:42:58 AM +07
#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H
#include "cglm/cglm.h"
#include "toolbox.h"
// clang-format off
#define COLOR_WHITE_INIT { 1.f, 1.f, 1.f, 1.f }
#define COLOR_WHITE (vec4) COLOR_WHITE_INIT

#define COLOR_BLACK_INIT { 0.f, 0.f, 0.f, 1.f } 
#define COLOR_BLACK (vec4) COLOR_BLACK_INIT

#define COLOR_BLUE_INIT { 0.f, 0.f, 1.f, 1.f }
#define COLOR_BLUE (vec4) COLOR_BLUE_INIT

#define COLOR_GREEN_INIT { 0.f, 1.f, 0.f, 1.f }
#define COLOR_GREEN (vec4) COLOR_GREEN_INIT

#define COLOR_RED_INIT { 1.f, 0.f, 0.f, 1.f }
#define COLOR_RED (vec4) COLOR_RED_INIT

#define COLOR_INIT(r, g, b, a)\
  { (float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f }

#define COLOR(r, g, b, a) ((vec4)COLOR_INIT(r, g, b, a))
// clang-format on

typedef struct IntRect
{
  int x, y, w, h;
} IntRect;

typedef struct UVs
{
  float u1, v1, u2, v2;
} UVs;

typedef struct Texture
{
  unsigned int handle; /* handle to opengl texture */
  int          width;  /* texture's width          */
  int          height; /* texture's height         */
} Texture;

typedef struct Sprite
{
  const Texture* texture;
  IntRect        rect; /* read only */
  float          u1;   /* read only */
  float          v1;   /* read only */
  float          u2;   /* read only */
  float          v2;   /* read only */
} Sprite;

typedef struct Vertex
{
  vec3 position;
  vec2 tex_coords;
  vec4 color;
} Vertex;

typedef enum TextAlignment
{
  TEXT_ALIGN_LEFT,
  TEXT_ALIGN_RIGHT,
  TEXT_ALIGN_CENTER
} TextAlignment;

typedef struct RenderStatistics
{
  u32 draw_call_count;
  u32 vertex_count;
} RenderStatistics;

/* shader use to drawing sprite */
typedef struct Shader
{
  /* program handle */
  unsigned int handle;

  /* where projection matrix uniform is loacated */
  int view_proj_matrix_loc;

  /* where time uniform is loacated */
  int time_loc;

} Shader;

typedef enum PixelFormat
{
  PIXEL_FORMAT_RGBA,
  PIXEL_FORMAT_BGRA
} PixelFormat;

/**
 * \brief load texture data form file
 * \return 0 if succeed
 */
int texture_load(Texture* texture, const char* file);

int texture_load_from_memory(Texture* texture, const u8* data, int width, int height, PixelFormat data_format);

void texture_init_empty(Texture* texture, int width, int height, PixelFormat data_format);

/* bind given texture to current opengl context */
void texture_bind(const Texture* texture);

/* unbind current texture */
void texture_unbind(const Texture* texture);

/* free texture data */
void texture_destroy(Texture* texture);

/* \brief set texture rect, null for entrie texture */
void sprite_set_rect(Sprite* region, const IntRect* rect);

/* \brief set texture and texture rect,
 *  set rect to NULL for entrie region */
void sprite_set_texture(Sprite* region, const Texture* texture, const IntRect* rect);

#define sprite_init sprite_set_texture

Sprite texture_region(const Texture* texture, const IntRect* rect);

/**
 * \brief create new shader for rendering quad
 * \return 0 if load succeed
 */
int shader_load(Shader* shader, const char* vs_file, const char* fs_file);

int shader_load_from_source(Shader* shader, const char* vs_source, const char* fs_source);

/**
 * \brief destroy given shader
 */
void shader_destroy(Shader* shader);

/**
 * \brief bind given shader to current gl context
 */
void shader_bind(const Shader* shader);

/**
 * unbind current shader
 */
void shader_unbind(const Shader* shader);

/* *
 * set projection matrix uniform
 */
void shader_upload_view_project_matrix(const Shader* shader, mat4 matrix);

void shader_upload_mat4(const Shader* shader, const char* name, mat4 matrix);

void shader_unload_float(const Shader* shader, const char* name, float afloat);

void shader_upload_int(const Shader* shader, const char* name, int aint);

void shader_upload_vec2(const Shader* shader, const char* name, const float v[2]);

void shader_upload_vec3(const Shader* shader, const char* name, const float v[3]);

void shader_upload_vec4(const Shader* shader, const char* name, const float v[4]);

#endif // GRAPHICS_TYPES_H
