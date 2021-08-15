#include "cglm/cglm.h"
#include "graphics.h"
#include "toolbox.h"
#include <cglm/vec4.h>

static Font sDefaultFont;

static float
text_width(const char* text, const Font* font, float scale)
{
  float       textWidth = 0;
  const char* iter      = text;
  u32         codepoint;
  while (*iter)
  {
    codepoint = (u32)*iter;
    textWidth += (float)font->glyphs[codepoint].advance[0] * scale;
  }

  return textWidth;
}

void
draw_text(const char* text, float x, float y, vec4 color)
{
  draw_text_ex(text, &sDefaultFont, x, y, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_textv(const char* text, vec2 position, vec4 color)
{
  draw_textv_ex(text, &sDefaultFont, position, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_text_ex(const char*   text,
             const Font*   font,
             float         x,
             float         y,
             float         scale,
             TextAlignment alignment,
             vec4          color)
{
  draw_textv_ex(text, font, (vec2){ x, y }, scale, alignment, color);
}

void
draw_textv_ex(const char*   text,
              const Font*   font,
              vec2          position,
              float         scale,
              TextAlignment alignment,
              vec4          color)
{
  // TODO: decode utf8
  u32         codepoint;
  const char* iter = text;
  vec2        drawPos;

  switch (alignment)
  {
  case TEXT_ALIGN_LEFT:
    drawPos[0] = position[0];
    break;
  case TEXT_ALIGN_RIGHT:
    drawPos[0] = position[0] - text_width(text, font, scale);
    break;
  case TEXT_ALIGN_CENTER:
    drawPos[0] = position[0] - text_width(text, font, scale) / 2.f;
    break;
  }

  while (*iter)
  {
    codepoint = (u32)*iter;
    if (codepoint == '\n')
    {
      drawPos[1] += 16.f; // TODO get character max height
      continue;
    }

    drawPos[0] = draw_codepoint(codepoint, font, drawPos, scale, color);
  }
}

float
draw_codepoint(u32 codepoint, const Font* font, vec2 position, float scale, vec4 color)
{
  const Glyph* glyph = &font->glyphs[codepoint];
  vec2         drawPos;
  vec2         size;

  drawPos[0] = position[0] + glyph->bearing[0] * scale;
  drawPos[1] = position[1] + glyph->bearing[1] * scale;

  size[0] = glyph->size[0] * scale;
  size[1] = glyph->size[1] * scale;
  draw_texture_region(&glyph->tex, position, size, color);

  return position[0] + (float)glyph->advance[0] * scale;
}
