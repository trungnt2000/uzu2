#include "font_loader.h"
#include "graphics/gl.h"
#include "toolbox/common.h"
#include "toolbox/log.h"
#include <ft2build.h>
#include <inttypes.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

static const int startChar = 32;
static const int endChar   = 126;

static FT_Library fontLibrary;

int
font_loader_init()
{
  FT_Error error = 0;
  if ((error = FT_Init_FreeType(&fontLibrary)) != 0)
    return error;
  return 0;
}

void
font_loader_shutdown()
{
  FT_Done_FreeType(fontLibrary);
}

int
font_face_load(FontFace* face, const char* facePath, unsigned faceIndex)
{
  FT_Error error = 0;
  FT_Face  fontFace;
  if ((error = FT_New_Face(fontLibrary, facePath, faceIndex, &fontFace)) != 0)
  {
    UZU_ERROR("Unable to load new face");
    return error;
  }
  *face = (void*)fontFace;
  return 0;
}

void
font_face_destroy(FontFace face)
{
  FT_Done_Face((FT_Face)face);
}

static int  generate_atlas(FT_Face fontFace, FontAtlas* atlas);
static void export_atlas(const FontAtlas* atlas);

int
font_atlas_load(FontAtlas* atlas, const char* fontPath, unsigned fontSize)
{
  FT_Face face;
  int     error = 0;
  if ((error = font_face_load(&face, fontPath, 0)) != 0)
    return error;

  if ((error = font_atlas_load_ex(atlas, face, fontSize)) != 0)
  {
    font_face_destroy(face);
    return error;
  }

  font_face_destroy(face);
  return 0;
}

int
font_atlas_load_ex(FontAtlas* atlas, FontFace face, unsigned fontSize)
{
  FT_Error error    = 0;
  FT_Face  fontFace = (FT_Face)face;

  const float    pt2pxRatio      = (float)4 / 3;
  const unsigned pixelHeight     = (unsigned)(fontSize * pt2pxRatio);
  atlas->charInfosLength         = endChar + startChar + 1;
  const size_t charInfoArraySize = sizeof(CharInfo) * atlas->charInfosLength;
  atlas->charInfos               = SDL_malloc(charInfoArraySize);
  SDL_memset(atlas->charInfos, 0, charInfoArraySize);

  if ((error = FT_Set_Pixel_Sizes(fontFace, 0, pixelHeight)) != 0)
    return error;
  atlas->pixelHeight = pixelHeight;

  FT_GlyphSlot glyph = fontFace->glyph;
  for (int c = startChar; c <= endChar; ++c)
  {
    if ((error = FT_Load_Char(fontFace, c, FT_LOAD_RENDER)) != 0)
    {
      UZU_ERROR("Unable to load character \'%c\' in face \"%s\", style \"%s\"",
                (char)c,
                fontFace->family_name,
                fontFace->style_name);
      continue;
    }

    atlas->charInfos[c].advance[0]    = glyph->metrics.horiAdvance >> 6;
    atlas->charInfos[c].advance[1]    = glyph->metrics.vertAdvance >> 6;
    atlas->charInfos[c].bitmapSize[0] = glyph->metrics.width >> 6;
    atlas->charInfos[c].bitmapSize[1] = glyph->metrics.height >> 6;
    printf("%c: %dx%d, %dx%d\n",
           (char)c,
           glyph->bitmap.width,
           glyph->bitmap.rows,
           atlas->charInfos[c].bitmapSize[0],
           atlas->charInfos[c].bitmapSize[1]);
    atlas->charInfos[c].bitmapBearing[0] = glyph->metrics.horiBearingX >> 6;
    atlas->charInfos[c].bitmapBearing[1] = glyph->metrics.horiBearingY >> 6;
  }
  error = generate_atlas(fontFace, atlas);

  if (error != 0)
  {
    return error;
  }
  export_atlas(atlas);

  return 0;
}

void
font_atlas_destroy(FontAtlas* atlas)
{
  SDL_free(atlas->charInfos);
  SDL_free(atlas->texture);
}

static void
export_atlas(const FontAtlas* atlas)
{
  FILE* f = fopen("test.atlas.raw", "wb");
  fwrite(&atlas->width, sizeof(atlas->width), 1, f);
  fwrite(&atlas->height, sizeof(atlas->height), 1, f);
  fwrite(atlas->texture,
         sizeof(unsigned char),
         (size_t)atlas->width * atlas->height,
         f);
  fwrite(&startChar, sizeof(startChar), 1, f);
  fwrite(&endChar, sizeof(endChar), 1, f);
  for (int i = startChar; i <= endChar; ++i)
  {
    const CharInfo* charInfo = &atlas->charInfos[i];
    fwrite(&charInfo->advance[0], sizeof(charInfo->advance[0]), 1, f);
    fwrite(&charInfo->advance[1], sizeof(charInfo->advance[1]), 1, f);
    fwrite(&charInfo->bitmapBearing[0],
           sizeof(charInfo->bitmapBearing[0]),
           1,
           f);
    fwrite(&charInfo->bitmapBearing[1],
           sizeof(charInfo->bitmapBearing[1]),
           1,
           f);
    fwrite(&charInfo->bitmapSize[0], sizeof(charInfo->bitmapSize[0]), 1, f);
    fwrite(&charInfo->bitmapSize[1], sizeof(charInfo->bitmapSize[1]), 1, f);
    fwrite(&charInfo->texBottomRight[0],
           sizeof(charInfo->texBottomRight[0]),
           1,
           f);
    fwrite(&charInfo->texBottomRight[1],
           sizeof(charInfo->texBottomRight[1]),
           1,
           f);
    fwrite(&charInfo->texTopLeft[0], sizeof(charInfo->texTopLeft[0]), 1, f);
    fwrite(&charInfo->texTopLeft[1], sizeof(charInfo->texTopLeft[1]), 1, f);
  }
  fclose(f);
}

static int
generate_atlas(FT_Face fontFace, FontAtlas* atlas)
{
  int glyphMaxWidth  = 0;
  int glyphMaxHeight = 0;
  int glyphCount     = 0;
  for (int i = startChar; i <= endChar; ++i)
  {
    glyphMaxWidth  = max(glyphMaxWidth, atlas->charInfos[i].bitmapSize[0]);
    glyphMaxHeight = max(glyphMaxHeight, atlas->charInfos[i].bitmapSize[1]);
    ++glyphCount;
  }
  atlas->glyphMaxWidth  = glyphMaxWidth;
  atlas->glyphMaxHeight = glyphMaxHeight;

  const unsigned required_area =
      (unsigned)(glyphMaxWidth * glyphMaxHeight * glyphCount);
  unsigned int texture_size = 1;
  while ((texture_size * texture_size) < required_area)
  {
    texture_size <<= 1u;
  };
  atlas->width  = texture_size;
  atlas->height = texture_size;

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RED,
               atlas->width,
               atlas->height,
               0,
               GL_RED,
               GL_UNSIGNED_BYTE,
               0);
  GLubyte zero = 0;
  glClearTexImage(tex, 0, GL_RED, GL_UNSIGNED_BYTE, &zero);
  int          offsetX    = 0;
  int          offsetY    = 0;
  const int    atlasWidth = atlas->width;
  FT_GlyphSlot glyph      = fontFace->glyph;

  for (int c = startChar; c <= endChar; c++)
  {
    if (FT_Load_Char(fontFace, c, FT_LOAD_RENDER))
      continue;
    if ((offsetX + (int)glyph->bitmap.width) >= atlasWidth)
    {
      offsetX = 0;
      offsetY += (int)glyphMaxHeight + 1;
    }

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    offsetX,
                    offsetY,
                    glyph->bitmap.width,
                    glyph->bitmap.rows,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    glyph->bitmap.buffer);

    atlas->charInfos[c].texTopLeft[0] = (float)offsetX / (float)atlas->width;
    atlas->charInfos[c].texTopLeft[1] = (float)offsetY / (float)atlas->height;
    atlas->charInfos[c].texBottomRight[0] =
        (offsetX + (float)glyph->bitmap.width) / (float)atlas->width;
    atlas->charInfos[c].texBottomRight[1] =
        (offsetY + (float)glyph->bitmap.rows) / (float)atlas->height;

    offsetX += glyph->bitmap.width + 1;
  }
  const size_t bufferSize = sizeof(GLubyte) * atlas->width * atlas->height;
  GLubyte*     texData    = SDL_malloc(bufferSize);
  SDL_memset(texData, 0, bufferSize);
  glGetTextureImage(tex, 0, GL_RED, GL_UNSIGNED_BYTE, bufferSize, texData);
  atlas->texture = texData;
  glBindTexture(GL_TEXTURE_2D, 0);
  glDeleteTextures(1, &tex);
  return 0;
}