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
  unsigned       charInfoLength;
  CharInfo*      charInfo;
} FontAtlas;

int font_loader_init(FontLibrary* library);
int font_loader_face_create(FontLibrary library,
                            const char* faceDir,
                            FontFace*   face);
int font_loader_atlas_create(FontLibrary library,
                             FontFace    face,
                             FontAtlas*  atlas,
                             unsigned    fontSize);

void font_loader_atlas_free(FontAtlas* atlas);
void font_loader_face_free(FontFace face);
void font_loader_shutdown(FontLibrary library);
#endif // !FONT_LOADER_H
