#ifndef TEXT_H
#define TEXT_H
#include "cglm/types.h"
#include "font_loader.h"

typedef struct TextVertex
{
  vec3 position;
  vec4 color;
  vec2 uv;
} TextVertex;

void text_indices_update(int length, unsigned* indices);

void text_line_update(const char*      string,
                      int              length,
                      const FontAtlas* atlas,
                      TextVertex*      vertices,
                      int              vertPerRec,
                      float            scale,
                      int              screenWidth,
                      int              screenHeigth,
                      const vec2       position);
#endif // !TEXT_H
