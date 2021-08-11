#include "font_loader.h"
#include "graphics/gl.h"
#include "toolbox/common.h"
#include "toolbox/log.h"
#include <ft2build.h>
#include <inttypes.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

static const int start_char = 32;
static const int end_char   = 126;

int
font_loader_init(FontLibrary* library)
{
  FT_Library fontLibrary;
  FT_Error   error = 0;
  if ((error = FT_Init_FreeType(&fontLibrary)) != 0)
    return error;
  *library = fontLibrary;
  return 0;
}

int
font_loader_face_create(FontLibrary library,
                        const char* faceDir,
                        FontFace*   face)
{
  FT_Error error = 0;
  FT_Face  fontFace;
  if ((error = FT_New_Face(library, faceDir, 0, &fontFace)) != 0)
  {
    UZU_ERROR("Unable to load new face");
    return error;
  }
  *face = (void*)fontFace;
  return 0;
}

static int  generate_atlas(FT_Face fontFace, FontAtlas* atlas);
static void export_atlas(const FontAtlas* atlas);

int
font_loader_atlas_create(FontLibrary library,
                         FontFace    face,
                         FontAtlas*  atlas,
                         unsigned    fontSize)
{
  FT_Error error    = 0;
  FT_Face  fontFace = (FT_Face)face;

  const float    pt2pxRatio      = (float)4 / 3;
  const unsigned pixelHeight     = (unsigned)(fontSize * pt2pxRatio);
  atlas->charInfoLength          = end_char + start_char + 1;
  const size_t charInfoArraySize = sizeof(CharInfo) * atlas->charInfoLength;
  atlas->charInfo                = SDL_malloc(charInfoArraySize);
  SDL_memset(atlas->charInfo, 0, charInfoArraySize);

  if ((error = FT_Set_Pixel_Sizes(fontFace, 0, pixelHeight)) != 0)
    return error;
  atlas->pixelHeight = pixelHeight;

  FT_GlyphSlot glyph = fontFace->glyph;
  for (int c = start_char; c <= end_char; ++c)
  {
    if ((error = FT_Load_Char(fontFace, c, FT_LOAD_RENDER)) != 0)
    {
      UZU_ERROR("Unable to load character \'%c\' in face \"%s\", style \"%s\"",
                (char)c,
                fontFace->family_name,
                fontFace->style_name);
      continue;
    }

    atlas->charInfo[c].advance[0]    = glyph->metrics.horiAdvance >> 6;
    atlas->charInfo[c].advance[1]    = glyph->metrics.vertAdvance >> 6;
    atlas->charInfo[c].bitmapSize[0] = glyph->metrics.width >> 6;
    atlas->charInfo[c].bitmapSize[1] = glyph->metrics.height >> 6;
    printf("%c: %dx%d, %dx%d\n",
           (char)c,
           glyph->bitmap.width,
           glyph->bitmap.rows,
           atlas->charInfo[c].bitmapSize[0],
           atlas->charInfo[c].bitmapSize[1]);
    atlas->charInfo[c].bitmapBearing[0] = glyph->metrics.horiBearingX >> 6;
    atlas->charInfo[c].bitmapBearing[1] = glyph->metrics.horiBearingY >> 6;
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
font_loader_face_free(FontFace face)
{
  FT_Done_Face((FT_Face)face);
}

void
font_loader_atlas_free(FontAtlas* atlas)
{
  SDL_free(atlas->charInfo);
  SDL_free(atlas->texture);
}

void
font_loader_shutdown(FontLibrary library)
{
  FT_Done_FreeType(library);
  // for (int font_idx = 0; font_idx < FONT_CNT; ++font_idx)
  //{
  //  for (int atlas_idx = 0; atlas_idx < FONT_ATLAS_SIZE; ++atlas_idx)
  //  {
  //    SDL_free(fontAtlas[font_idx][atlas_idx].texture);
  //  }
  //}
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
  fwrite(&start_char, sizeof(start_char), 1, f);
  fwrite(&end_char, sizeof(end_char), 1, f);
  for (int i = start_char; i <= end_char; ++i)
  {
    const CharInfo* charInfo = &atlas->charInfo[i];
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
  for (int i = start_char; i <= end_char; ++i)
  {
    glyphMaxWidth  = max(glyphMaxWidth, atlas->charInfo[i].bitmapSize[0]);
    glyphMaxHeight = max(glyphMaxHeight, atlas->charInfo[i].bitmapSize[1]);
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

  for (int c = start_char; c <= end_char; c++)
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

    atlas->charInfo[c].texTopLeft[0] = (float)offsetX / (float)atlas->width;
    atlas->charInfo[c].texTopLeft[1] = (float)offsetY / (float)atlas->height;
    atlas->charInfo[c].texBottomRight[0] =
        (offsetX + (float)glyph->bitmap.width) / (float)atlas->width;
    atlas->charInfo[c].texBottomRight[1] =
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