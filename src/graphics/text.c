#include "graphics/text.h"
#include "text_format.h"
#include <cglm/vec4.h>
#include <stdio.h>

void
text_indices_update(int length, unsigned* indices)
{
  for (int i = 0; i < length; ++i)
  {
    const int recOffset  = i * TEXT_IDX_PER_RECT;
    const int vertOffset = i * TEXT_VERT_PER_RECT;
    unsigned* idxRec     = indices + recOffset;
    idxRec[0]            = 0 + vertOffset;
    idxRec[1]            = 2 + vertOffset;
    idxRec[2]            = 1 + vertOffset;
    idxRec[3]            = 0 + vertOffset;
    idxRec[4]            = 2 + vertOffset;
    idxRec[5]            = 3 + vertOffset;
  }
}

static inline void
text_verticies_update(int             codePointIdx,
                      const CharInfo* charInfo,
                      TextVertex*     vertRec,
                      const vec4      color,
                      const float     scale,
                      const float     scaleW,
                      const float     scaleH,
                      float*          xpos,
                      float*          ypos)
{
  const float deltaX = charInfo->bitmapBearing[0] * scaleW;
  const float deltaY =
      (charInfo->bitmapSize[1] - charInfo->bitmapBearing[1]) * scaleH;
  const float deltaW           = charInfo->bitmapSize[0] * scaleW;
  const float deltaH           = charInfo->bitmapSize[1] * scaleH;
  const float xW               = *xpos + (deltaX * scale);
  const float yW               = *ypos - (deltaY * scale);
  const float w                = deltaW * scale;
  const float h                = deltaH * scale;
  const float deltaAdvanceHori = charInfo->advance[0] * scaleW;
  // const float deltaAdvanceVert = charInfo->advance[1] * scaleH;

  vertRec[0].position[0] = xW;
  vertRec[0].position[1] = yW;
  vertRec[0].position[2] = 0.f;
  vertRec[0].color[0]    = color[0]; //
  vertRec[0].color[1]    = color[1];
  vertRec[0].color[2]    = color[2];
  vertRec[0].color[3]    = color[3];
  vertRec[0].uv[0]       = charInfo->texTopLeft[0];
  vertRec[0].uv[1]       = charInfo->texBottomRight[1];

  vertRec[1].position[0] = xW;
  vertRec[1].position[1] = h + yW;
  vertRec[1].position[2] = 0.f;
  vertRec[1].color[0]    = color[0];
  vertRec[1].color[1]    = color[1];
  vertRec[1].color[2]    = color[2];
  vertRec[1].color[3]    = color[3];
  vertRec[1].uv[0]       = charInfo->texTopLeft[0];
  vertRec[1].uv[1]       = charInfo->texTopLeft[1];

  vertRec[2].position[0] = xW + w;
  vertRec[2].position[1] = h + yW;
  vertRec[2].position[2] = 0.f;
  vertRec[2].color[0]    = color[0];
  vertRec[2].color[1]    = color[1];
  vertRec[2].color[2]    = color[2];
  vertRec[2].color[3]    = color[3];
  vertRec[2].uv[0]       = charInfo->texBottomRight[0];
  vertRec[2].uv[1]       = charInfo->texTopLeft[1];

  vertRec[3].position[0] = xW + w;
  vertRec[3].position[1] = yW;
  vertRec[3].position[2] = 0.f;
  vertRec[3].color[0]    = color[0];
  vertRec[3].color[1]    = color[1];
  vertRec[3].color[2]    = color[2];
  vertRec[3].color[3]    = color[3];
  vertRec[3].uv[0]       = charInfo->texBottomRight[0];
  vertRec[3].uv[1]       = charInfo->texBottomRight[1];

  *xpos += deltaAdvanceHori * scale;
}

void
text_update(TextFormatContext* ctx,
                 const FontAtlas* atlas,
                 TextVertex*      vertices,
                 float            scale,
                 int              screenWidth,
                 int              screenHeigth,
                 const vec2       position)
{
  float       xpos         = position[0];
  float       ypos         = position[1];
  const float scaleW       = 2.f / screenWidth;
  const float scaleH       = 2.f / screenHeigth;

  int         codePointIdx = 0;
  int         colorIdx     = 0;
  TextVertex* vertRec;
  vec4        currentColor;
  glm_vec4_copy(ctx->defaultColor, currentColor);

  for (int i = 0; i < ctx->unitsSize; ++i)
  {
    switch (ctx->units[i])
    {
    case TFTCodePoint:
      // printf("Codepoint: %c\n", (char)ctx->codePoints[codePointIdx]);
      vertRec = vertices + ((long long)codePointIdx * TEXT_VERT_PER_RECT);
      text_verticies_update(codePointIdx,
                            &atlas->charInfos[ctx->codePoints[codePointIdx]],
                            vertRec,
                            currentColor,
                            scale,
                            scaleW,
                            scaleH,
                            &xpos,
                            &ypos);
      ++codePointIdx;
      break;
    case TFTChangeColor:
      /*printf("Color: %f, %f, %f, %f\n",
             ctx->colors[colorIdx][0],
             ctx->colors[colorIdx][1],
             ctx->colors[colorIdx][2],
             ctx->colors[colorIdx][3]);*/
      glm_vec4_copy(ctx->colors[colorIdx], currentColor);
      ++colorIdx;
      break;
    case TFTNewLine:
      // printf("Newline\n");
      break;
    }
  }
}