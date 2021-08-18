#include "graphics/font.h"
#include "graphics/gl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define FIRST_CODEPOINT 0x00
#define LAST_CODEPOINT 0xff

static FT_Library sFTLibrary;

int
font_loader_init(void)
{
  int error;

  error = FT_Init_FreeType(&sFTLibrary);
  if (error)
    return -1;
  return 0;
}

void
font_loader_shutdown(void)
{
  FT_Done_FreeType(sFTLibrary);
  sFTLibrary = NULL;
}

static void
convert_8bpp_to_32bpp(const u8* src, u8* dst, int length)
{
  int i = 0;
  while (i < length)
  {
    u8 c = src[i];
    dst[0] = c;
    dst[1] = c;
    dst[2] = c;
    dst[3] = c;
    i++;
    dst += 4;
  }
}

int
font_load(Font* font, const char* file, u32 size)
{
  FT_Face      face = NULL;
  int          error;
  u32          glyphIndex;
  int          status = 0;
  FT_GlyphSlot glyph;
  int          maxGlyphWidth, maxGlyphHeight;
  int          x;
  GLuint       tex;
  const u32    numGlyphs = LAST_CODEPOINT - FIRST_CODEPOINT + 1;
  u32          atlasWidth;
  u32          atlasHeight;
  u8*          buff;

  error = FT_New_Face(sFTLibrary, file, 0, &face);

  if (error)
  {
    UZU_ERROR("Failed to create font\n");
    return -1;
  }

  error = FT_Set_Pixel_Sizes(face, size, size);
  if (error)
  {
    UZU_ERROR("Failed to set font face size\n");
    status = -1;
    goto cleanup;
  }

  glyph          = face->glyph;
  maxGlyphWidth  = 0;
  maxGlyphHeight = 0;
  for (u32 codepoint = FIRST_CODEPOINT; codepoint < LAST_CODEPOINT; ++codepoint)
  {
    glyphIndex = FT_Get_Char_Index(face, codepoint);

    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER) != 0)
    {
      printf("Failed to load character \'%x\'\n", codepoint);
      continue;
    }
    maxGlyphWidth  = max((int)glyph->bitmap.width, maxGlyphWidth);
    maxGlyphHeight = max((int)glyph->bitmap.rows, maxGlyphHeight);
  }

  atlasWidth  = maxGlyphWidth * numGlyphs;
  atlasHeight = maxGlyphHeight;

  buff = SDL_malloc((sizeof *buff) * maxGlyphWidth * maxGlyphHeight * 4);

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  x                  = 0;
  font->atlas.width  = atlasWidth;
  font->atlas.height = atlasHeight;
  font->atlas.handle = tex;
  font->glyphCnt     = numGlyphs;
  font->glyphs       = SDL_malloc(sizeof(*font->glyphs) * font->glyphCnt);
  int glyphWidth, glyphHeight;
  for (u32 i = FIRST_CODEPOINT; i < LAST_CODEPOINT; i++)
  {
    glyphIndex = FT_Get_Char_Index(face, i);
    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER))
      continue;

    glyphWidth  = (int)glyph->bitmap.width;
    glyphHeight = (int)glyph->bitmap.rows;

    convert_8bpp_to_32bpp(glyph->bitmap.buffer, buff, glyphWidth * glyphHeight);

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, glyphWidth, glyphHeight, GL_RGBA, GL_UNSIGNED_BYTE, buff);

    texture_region_set_texture(&font->glyphs[i].tex,
                               &font->atlas,
                               &(IntRect){ x, 0, glyphWidth, glyphHeight });
    font->glyphs[i].advance[0] = (float)(glyph->metrics.horiAdvance >> 6);
    font->glyphs[i].advance[1] = (float)(glyph->metrics.vertAdvance >> 6);
    font->glyphs[i].bearing[0] = (float)(glyph->bitmap_left);
    font->glyphs[i].bearing[1] = (float)(glyph->bitmap_top);
    font->glyphs[i].size[0]    = (float)glyphWidth;
    font->glyphs[i].size[1]    = (float)glyphHeight;
    font->glyphMaxWidth = maxGlyphWidth;
    font->glyphMaxHeight = maxGlyphHeight;

    x += maxGlyphWidth;
  }
cleanup:
  FT_Done_Face(face);
  SDL_free(buff);
  return status;
}

void
font_destroy(Font* font)
{
  SDL_free(font->glyphs);
  texture_destroy(&font->atlas);
}
