#ifndef FONT_LOADER_H
#define FONT_LOADER_H
#include "cglm/types.h"

typedef void* FontLibrary;
typedef void* FontFace;
typedef int   ivec2[2];

typedef struct CharInfo
{
  ivec2 advance;
  ivec2 bitmapSize;
  ivec2 bitmapBearing;
  vec2  texTopLeft;
  vec2  texBottomRight;
} CharInfo;

typedef struct FontAtlas
{
  FontFace       fontFace;
  unsigned char* texture;
  int            height;
  int            width;
  int            pixelHeight;
  int            glyphMaxWidth;
  int            glyphMaxHeight;
  unsigned       charInfosLength;
  CharInfo*      charInfos;
} FontAtlas;

int  font_loader_init();
void font_loader_shutdown();

int  font_face_load(FontFace* face, const char* fontPath, unsigned faceIndex);
void font_face_destroy(FontFace face);

int  font_atlas_load(FontAtlas* atlas, const char* fontPath, unsigned fontSize);
int  font_atlas_load_ex(FontAtlas* atlas, FontFace face, unsigned fontSize);
void font_atlas_destroy(FontAtlas* atlas);

#endif // !FONT_LOADER_H
