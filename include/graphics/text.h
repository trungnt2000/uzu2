#ifndef TEXT_H
#define TEXT_H

#include "cglm/types.h"
#include "font_loader.h"
#include "text_format.h"


#define TEXT_VERT_PER_RECT 4
#define TEXT_IDX_PER_RECT 6

typedef struct TextVertex
{
  vec3 position;
  vec4 color;
  vec2 uv;
} TextVertex;

void text_indices_update(int length, unsigned* indices);

void text_update(TextFormatContext* ctx,
                 const FontAtlas*   atlas,
                 TextVertex*        vertices,
                 float              scale,
                 int                screenWidth,
                 int                screenHeigth,
                 const vec2         position);
#endif // !TEXT_H
