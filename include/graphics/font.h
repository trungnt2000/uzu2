//ngotrung Tue 17 Aug 2021 11:23:00 AM +07
#ifndef FONT_H
#define FONT_H
#include "graphics/types.h"

typedef struct Glyph
{
  Sprite sprite;
  vec2          advance;
  vec2          size;
  vec2          bearing;
} Glyph;

typedef struct Font
{
  Texture atlas;
  Glyph*  glyphs;
  u32     glyph_cnt;
  float   max_glyph_width;
  float   max_glyph_height;
} Font;

int font_load(Font* font, const char* file, u32 size);

void font_destroy(Font* font);

int font_loader_init(void);

void font_loader_shutdown(void);

#endif //FONT_H
