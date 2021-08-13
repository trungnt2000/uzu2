#include "graphics/text.h"

void
text_indices_update(int length, unsigned* indices)
{
  for (int i = 0; i < length; ++i)
  {
    const int recOffset  = i * 6;
    const int vertOffset = i * 4;
    unsigned* idxRec     = indices + recOffset;
    idxRec[0]            = 0 + vertOffset;
    idxRec[1]            = 2 + vertOffset;
    idxRec[2]            = 1 + vertOffset;
    idxRec[3]            = 0 + vertOffset;
    idxRec[4]            = 2 + vertOffset;
    idxRec[5]            = 3 + vertOffset;
  }
}

void
text_line_update(const char*      string,
                 int              length,
                 const FontAtlas* atlas,
                 TextVertex*      vertices,
                 int              vertPerRec,
                 float            scale,
                 int              screenWidth,
                 int              screenHeigth,
                 const vec2       position)
{
  float xpos = position[0];
  float ypos = position[1];

  for (int i = 0; i < length; ++i)
  {
    TextVertex*     vertRec  = vertices + ((long long)i * vertPerRec);
    const CharInfo* charInfo = &atlas->charInfos[*(string + i)];

    const float     scaleW   = 2.f / screenWidth;
    const float     scaleH   = 2.f / screenHeigth;
    const float     deltaX   = charInfo->bitmapBearing[0] * scaleW;
    const float     deltaY =
        (charInfo->bitmapSize[1] - charInfo->bitmapBearing[1]) * scaleH;
    const float deltaW           = charInfo->bitmapSize[0] * scaleW;
    const float deltaH           = charInfo->bitmapSize[1] * scaleH;
    const float xW               = xpos + (deltaX * scale);
    const float yW               = ypos - (deltaY * scale);
    const float w                = deltaW * scale;
    const float h                = deltaH * scale;
    const float deltaAdvanceHori = charInfo->advance[0] * scaleW;
    // const float deltaAdvanceVert = charInfo->advance[1] * scaleH;

    vertRec[0].position[0] = xW;
    vertRec[0].position[1] = yW;
    vertRec[0].position[2] = 0.f;
    vertRec[0].color[0]    = 1.f; //
    vertRec[0].color[1]    = 1.f;
    vertRec[0].color[2]    = 1.f;
    vertRec[0].color[3]    = 1.f;
    vertRec[0].uv[0]       = charInfo->texTopLeft[0];
    vertRec[0].uv[1]       = charInfo->texBottomRight[1];

    vertRec[1].position[0] = xW;
    vertRec[1].position[1] = h + yW;
    vertRec[1].position[2] = 0.f;
    vertRec[1].color[0]    = 1.f;
    vertRec[1].color[1]    = 1.f;
    vertRec[1].color[2]    = 1.f;
    vertRec[1].color[3]    = 1.f;
    vertRec[1].uv[0]       = charInfo->texTopLeft[0];
    vertRec[1].uv[1]       = charInfo->texTopLeft[1];

    vertRec[2].position[0] = xW + w;
    vertRec[2].position[1] = h + yW;
    vertRec[2].position[2] = 0.f;
    vertRec[2].color[0]    = 1.f;
    vertRec[2].color[1]    = 1.f;
    vertRec[2].color[2]    = 1.f;
    vertRec[2].color[3]    = 1.f;
    vertRec[2].uv[0]       = charInfo->texBottomRight[0];
    vertRec[2].uv[1]       = charInfo->texTopLeft[1];

    vertRec[3].position[0] = xW + w;
    vertRec[3].position[1] = yW;
    vertRec[3].position[2] = 0.f;
    vertRec[3].color[0]    = 1.f;
    vertRec[3].color[1]    = 1.f;
    vertRec[3].color[2]    = 1.f;
    vertRec[3].color[3]    = 1.f;
    vertRec[3].uv[0]       = charInfo->texBottomRight[0];
    vertRec[3].uv[1]       = charInfo->texBottomRight[1];

    xpos += deltaAdvanceHori * scale;
  }
}