#include "font_loader.h"
#include "graphics/gl.h"
#include <string.h>

typedef struct point
{
  GLfloat x;
  GLfloat y;
  GLfloat s;
  GLfloat t;
} point;

void
render_text(const FontAtlas* atlas,
            GLuint           indexBuffer,
            const char*      text,
            float            x,
            float            y,
            float            sx,
            float            sy)
{

  //point coords[6 * 5];

  //int n = 0;

  //for (const char* p = text; *p; p++)
  //{
  //  float x2 = x + atlas->charInfo[*p].bitmapLeft * sx;
  //  float y2 = -y - atlas->charInfo[*p].bitmapTop * sy;
  //  float w  = atlas->charInfo[*p].bitmapWidth * sx;
  //  float h  = atlas->charInfo[*p].bitmapHeight * sy;

  //  /* Advance the cursor to the start of the next character */
  //  x += atlas->charInfo[*p].advanceX * sx;
  //  y += atlas->charInfo[*p].advanceY * sy;

  //  /* Skip glyphs that have no pixels */
  //  if (!w || !h)
  //    continue;

  //  coords[n++] = (point){ x2, -y2, atlas->charInfo[*p].textureX, 0 };
  //  coords[n++] = (point){ x2 + w,
  //                         -y2,
  //                         atlas->charInfo[*p].textureX +
  //                             atlas->charInfo[*p].bitmapWidth / atlas->width,
  //                         0 };
  //  coords[n++] = (point){
  //    x2,
  //    -y2 - h,
  //    atlas->charInfo[*p].textureX,
  //    atlas->charInfo[*p].bitmapHeight / atlas->glyphMaxHeight
  //  }; // remember: each glyph occupies a different amount of vertical space
  //  coords[n++] = (point){ x2 + w,
  //                         -y2,
  //                         atlas->charInfo[*p].textureX +
  //                             atlas->charInfo[*p].bitmapWidth / atlas->width,
  //                         0 };
  //  coords[n++] =
  //      (point){ x2,
  //               -y2 - h,
  //               atlas->charInfo[*p].textureX,
  //               atlas->charInfo[*p].bitmapWidth / atlas->glyphMaxHeight };
  //  coords[n++] =
  //      (point){ x2 + w,
  //               -y2 - h,
  //               atlas->charInfo[*p].textureX +
  //                   atlas->charInfo[*p].bitmapWidth / atlas->width,
  //               atlas->charInfo[*p].bitmapWidth / atlas->glyphMaxHeight };
  //}
  ////glGenBuffers(1, &index_buffer);
  //glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
  //glBufferData(GL_ARRAY_BUFFER, sizeof coords, coords, GL_DYNAMIC_DRAW);
  //glDrawArrays(GL_TRIANGLES, 0, n);
  //glBindVertexArray(0);
  //glBindTexture(GL_TEXTURE_2D, 0);
}