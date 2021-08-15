#include "graphics.h"

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
}

int
font_load(Font* font, const char* file, u32 size)
{
  FT_Face      face = NULL;
  int          error;
  u32          glyphIndex;
  int          status = 0;
  FT_GlyphSlot glyph;
  int          texWidth, texHeight;
  int          x;

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

  glyph     = face->glyph;
  texWidth  = 0;
  texHeight = 0;
  for (u32 codepoint = FIRST_CODEPOINT; codepoint < LAST_CODEPOINT; ++codepoint)
  {
    glyphIndex = FT_Get_Char_Index(face, codepoint);

    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER) != 0)
    {
      printf("Failed to load character \'%c\'\n", (char)codepoint);
      continue;
    }
    texWidth += glyph->bitmap.width;
    texHeight = max((int)glyph->bitmap.rows, texHeight);
  }

  GLuint tex;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

  x                  = 0;
  font->atlas.width  = (u32)texWidth;
  font->atlas.height = (u32)texHeight;
  font->atlas.handle = tex;
  font->glyphCnt     = LAST_CODEPOINT - FIRST_CODEPOINT + 1;
  font->glyphs       = SDL_malloc(sizeof(*font->glyphs) * font->glyphCnt);
  for (u32 i = FIRST_CODEPOINT; i < LAST_CODEPOINT; i++)
  {
    if (FT_Load_Char(face, i, FT_LOAD_RENDER))
      continue;

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    x,
                    0,
                    (int)glyph->bitmap.width,
                    (int)glyph->bitmap.rows,
                    GL_ALPHA,
                    GL_UNSIGNED_BYTE,
                    glyph->bitmap.buffer);

    font->glyphs[i].tex =
        texture_region(&font->atlas, &(IntRect){ x, 0, (int)glyph->bitmap.width, (int)glyph->bitmap.rows });
    font->glyphs[i].advance[0] = (float)(glyph->metrics.horiAdvance >> 6);
    font->glyphs[i].advance[1] = (float)(glyph->metrics.vertAdvance >> 6);
    font->glyphs[i].bearing[0] = (float)(glyph->metrics.vertBearingX >> 6);
    font->glyphs[i].bearing[1] = (float)(glyph->metrics.vertBearingY >> 6);
    font->glyphs[i].size[0]    = (float)(glyph->metrics.width);
    font->glyphs[i].size[1]    = (float)(glyph->metrics.height);

    x += glyph->bitmap.width;
  }
cleanup:
  FT_Done_Face(face);
  return status;
}

void
font_destroy(Font* font)
{
  SDL_free(font->glyphs);
  texture_destroy(&font->atlas);
}
