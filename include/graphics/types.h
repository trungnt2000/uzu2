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
// clang-format on

typedef struct IntRect
{
  int x, y, w, h;
} IntRect;

typedef struct Texture
{
  unsigned int handle; /* handle to opengl texture */
  int          width;  /* texture's width          */
  int          height; /* texture's height         */
} Texture;

typedef struct TextureRegion
{
  const Texture* texture;
  IntRect        rect; /* read only */
  float          u1;      /* read only */
  float          v1;      /* read only */
  float          u2;      /* read only */
  float          v2;      /* read only */
} TextureRegion;

typedef struct Vertex
{
  vec3 position;
  vec2 texCoord;
  vec4 color;
} Vertex;

typedef enum TextAlignment
{
  TEXT_ALIGN_LEFT,
  TEXT_ALIGN_RIGHT,
  TEXT_ALIGN_CENTER
} TextAlignment;

typedef enum TextWrap
{
  TEXT_WRAP_NORMAL,
  TEXT_WRAP_WORD
} TextWrap;

/* shader use to drawing sprite */
typedef struct SpriteShader
{
  /* program handle */
  unsigned int handle;

  /* where projection matrix uniform is loacated */
  int uProjMatLocation;

} SpriteShader;

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

int texture_load_from_memory(Texture* texture, const u8* data, int width, int height, PixelFormat dataFormat);

/* bind given texture to current opengl context */
void texture_bind(const Texture* texture);

/* unbind current texture */
void texture_unbind(const Texture* texture);

/* free texture data */
void texture_destroy(Texture* texture);

/* \brief set texture rect, null for entrie texture */
void texture_region_set_rect(TextureRegion* region, const IntRect* rect);

/* \brief set texture and texture rect,
 *  set rect to NULL for entrie region */
void texture_region_set_texture(TextureRegion* region, const Texture* texture, const IntRect* rect);

#define texture_region_init texture_region_set_texture

TextureRegion texture_region(const Texture* texture, const IntRect* rect);

/**
 * \brief create new shader for rendering quad
 * \return 0 if load succeed
 */
int sprite_shader_load(SpriteShader* shader);

/**
 * \brief destroy given shader
 */
void sprite_shader_destroy(SpriteShader* shader);

/**
 * \brief bind given shader to current gl context
 */
void sprite_shader_bind(SpriteShader* shader);

/**
 * unbind current shader
 */
void sprite_shader_unbind(SpriteShader* shader);

/* *
 * set projection matrix uniform
 */
void sprite_shader_uniform_projmat(SpriteShader* shader, mat4 projMat);

#endif // GRAPHICS_TYPES_H
