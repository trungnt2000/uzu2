#ifndef TEXT_H
#define TEXT_H

#include "cglm/types.h"
#include "font_loader.h"
#include "text_format.h"

#define TEXT_VERT_PER_GLYPH 4
#define TEXT_IDX_PER_GLYPH 6

typedef struct TextVertex
{
  vec3 position;
  vec4 color;
  vec2 uv;
} TextVertex;

int  text_renderer_init(u32              maxGlyphs,
                        const FontAtlas* atlas,
                        const vec4       defaultTextColor,
                        unsigned         screenWidth,
                        unsigned         screenHeigth);
void text_renderer_shutdown();
void text_batch_begin();
void text_batch_end();

void
draw_text(const char* text, float x, float y, const vec4 color, float scale);

void text_update(TextFormatContext* ctx,
                 const FontAtlas*   atlas,
                 TextVertex*        vertices,
                 unsigned*          indices,
                 float              scale,
                 int                screenWidth,
                 int                screenHeigth,
                 const vec2         position);
#endif // !TEXT_H
