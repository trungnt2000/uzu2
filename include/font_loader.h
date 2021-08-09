#ifndef FONT_LOADER_H
#define FONT_LOADER_H
#include "cglm/types.h"
#include "cglm/vec3.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef enum FontFace
{
  FontDefault,
  FontPaletteMosaic,
  FontRoboto,
  FONT_CNT
} FontFace;

typedef struct CharInfo
{
  vec2 advance;
  vec2 bitmapSize;
  vec2 bitmapBearing;
  vec2 texTopLeft;
  vec2 texBottomRight;
} CharInfo;

typedef struct FontAtlas
{
  unsigned char* texture;
  unsigned       height;
  unsigned       width;
  unsigned       pixelHeight;
  CharInfo       charInfo[128];
} FontAtlas;

int font_loader_init();

int
font_loader_load(FontFace face, unsigned atlasSizeStart, unsigned atlasSizeEnd);
const FontAtlas* font_loader_get_atlas(FontFace face, unsigned atlasIndex);
void             font_loader_destroy();
#endif // !FONT_LOADER_H
