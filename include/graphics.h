// ngotrung Wed 16 Jun 2021 08:16:56 PM +07
#ifndef GRAPHICS
#define GRAPHICS
#include "toolbox.h"
#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#include "cglm/cglm.h"

// clang-format off
#define COLOR_WHITE_INIT { 1.f, 1.f, 1.f, 1.f }
#define COLOR_WHITE (vec4) COLOR_WHITE_INIT

#define COLOR_BLACK_INIT { 0.f, 0.f, 0.f, 1.f } 
#define COLOR_BLACK (vec4) COLOR_BLACK_INIT
// clang-format on

typedef struct IntRect
{
  u32 x, y, w, h;
} IntRect;

typedef struct Texture
{
  GLuint handle; /* handle to opengl texture */
  u32    width;  /* texture's width          */
  u32    height; /* texture's height         */
} Texture;

/**
 * \brief load texture data form file
 * \return 0 if succeed
 */
int texture_load(Texture* texture, const char* file);

/* bind given texture to current opengl context */
void texture_bind(const Texture* texture);

/* free texture data */
void texture_free(Texture* texture);

typedef struct TextureRegion
{
  const Texture* texture;
  IntRect        texRect; /* read only */
  float          u1;      /* read only */
  float          v1;      /* read only */
  float          u2;      /* read only */
  float          v2;      /* read only */
} TextureRegion;

/* \brief set texture rect, null for entrie texture */
void texture_region_set_rect(TextureRegion* texegion, const IntRect* rect);

/* \brief set texture and texture rect,
 *  set rect to NULL for entrie region */
void texture_region_set_texture(TextureRegion* texRegion,
                                const Texture* texture,
                                const IntRect* rect);

typedef struct Vertex
{
  vec3 position;
  vec2 texCoord;
  vec4 color;
} Vertex;

/**
 * helper function to create new shader program
 * with given vertex shader and fragment shader file
 * output program is set via outProgram param
 * \return 0 if succeed -1 otherwise
 */
int create_shader(const char* vsFile, const char* fsFile, GLuint* outProgram);

/* Quad Rendering State*/
typedef struct SpriteRS SpriteRS;

/* create new sprite render state */
SpriteRS* sprite_render_state_create(u32 maxSprites);

/* destroy sprite render state */
void sprite_render_state_destroy(SpriteRS* rs);

/* prepare draw sprite batch */
void begin_draw_sprite(SpriteRS* rs);

/* \brief end sprite batch flush all draw command to gpu */
void end_draw_sprite(SpriteRS* rs);

/* \brief draw sprite, draw commands is send to gpu by
 *  changing texture or by calling end_draw_sprite */
void draw_sprite(SpriteRS*            rs,
                 vec2                 size,
                 vec2                 center,
                 vec4                 color,
                 float                depth,
                 const TextureRegion* textureRegion,
                 mat3                 transformMatrix);

/* shader use to drawing sprite */
typedef struct
{
  /* program handle */
  GLuint handle;

  /* where projection matrix uniform is loacated */
  int uProjMatLocation;

} SpriteShader;

/**
 * \brief create new shader for rendering quad
 * \return 0 if load succeed
 */
int sprite_shader_load(SpriteShader* shader);

/* destroy given shader */
void sprite_shader_destroy(SpriteShader* shader);

/* bind given shader to current gl context */
void sprite_shader_bind(SpriteShader* shader);

/* set projection matrix uniform */
void sprite_shader_uniform_projmat(SpriteShader* shader, mat4 projMat);

typedef struct Animation
{
  TextureRegion* frames;
  int            frameCnt;
  float          frameDuration;
} Animation;

typedef struct AnimationTemplate
{
  const Texture* texture;
  int            xOffset;
  int            yOffset;
  int            spriteWidth;
  int            spriteHeight;
  int            columnCount;
  int            rowCount;
  float          frameDuration;
} AnimationTemplate;

void anim_init_tmpl(Animation* anim, const AnimationTemplate* tmpl);
void anim_destroy(Animation* anim);
const TextureRegion* anim_get_frame(const Animation* anim, float elapsedTime);
#endif // GRAPHICS
