// ngotrung Tue 17 Aug 2021 10:43:11 AM +07
#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H
#include "graphics/font.h"
#include "graphics/types.h"

typedef struct RenderStatistics
{
  u32 drawCall;
  u32 verticeCount;
} RenderStatistics;

/* prepare draw sprite batch */
void sprite_batch_begin(void);

/**
 * \brief end sprite batch flush all draw command to gpu
 */
void sprite_batch_end(void);

/**
 * \brief draw sprite, draw commands is send to gpu by
 *        changing texture or by calling end_draw_sprite
 * \param size sprite's size
 * \param color sprite's color
 * \param depth effect draw order of sprite
 * \param tex texture use to draw given sprite
 * \param tx use to transform given sprite
 */
void
draw_texture_region_w_tx(const TextureRegion* tex, vec2 size, vec2 center, vec4 color, float depth, mat3 tx);

void draw_texture_region(const TextureRegion* tex, vec2 position, vec2 size, vec4 color);

void draw_text(const char* text, float x, float y, vec4 color);

void draw_textv(const char* text, vec2 position, vec4 color);

void draw_text_ex(const char*   text,
                  const Font*   font,
                  float         x,
                  float         y,
                  float         scale,
                  TextAlignment alignment,
                  vec4          color);

void draw_textv_ex(const char*   text,
                   const Font*   font,
                   vec2          position,
                   float         scale,
                   TextAlignment alignment,
                   vec4          color);

void draw_text_boxed(const char* text, vec4 box, vec4 color);

void draw_text_boxed_ex(const char*   text,
                        const Font*   font,
                        vec2          position,
                        vec2          size,
                        float         scale,
                        TextAlignment alignment,
                        TextWrap      wrap,
                        vec4          color);

void draw_codepoint(u32 codepoint, const Font* font, vec2 position, float scale, vec4 color);
/**
 * \brief init sprite renderer
 *
 * \param maxSprites how many sprites can batch together in one draw call
 */
void sprite_renderer_init(u32 maxSprites);

/**
 * \brief shutdown sprite renderer release all dynamic allocation
 */
void sprite_renderer_shutdown(void);

void sprite_renderer_query_statistics(RenderStatistics* statistics);

#endif // GRAPHICS_RENDERER_H
