// ngotrung Tue 17 Aug 2021 10:43:11 AM +07
#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H
#include "graphics/font.h"
#include "graphics/types.h"


void text_renderer_init(u32 bufferSize);

void text_renderer_shutdown(void);

void begin_text(const Shader* used_shader, mat4 used_mvp_matrix);

void end_text(void);

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
                        vec4          color);


void text_renderer_query_statistics(RenderStatistics* statistics);

#endif // GRAPHICS_RENDERER_H
