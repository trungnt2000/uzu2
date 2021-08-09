#include "font_loader.h"
#include "graphics/gl.h"
#include "toolbox/common.h"
#include "toolbox/log.h"

#define FONT_ASSET_DIR "res/font"
#define FONT_ATLAS_SIZE 52
static const int start_char = 32;
static const int end_char   = 127;

const char* fontDirs[FONT_CNT] = {
  [FontDefault]       = FONT_ASSET_DIR "/font.TTF",
  [FontPaletteMosaic] = FONT_ASSET_DIR "/PaletteMosaic-Regular.ttf",
  [FontRoboto]        = FONT_ASSET_DIR "/Roboto-Bold.ttf"
};

static FT_Library fontLibrary;
static FT_Face    faceList[FONT_CNT];
static FontAtlas  fontAtlas[FONT_CNT][FONT_ATLAS_SIZE];

int
font_loader_init()
{
  FT_Error error = 0;
  if ((error = FT_Init_FreeType(&fontLibrary)) != 0)
    return error;

  memset(fontAtlas, 0, sizeof(FontAtlas) * FONT_CNT * FONT_ATLAS_SIZE);
  memset(faceList, 0, sizeof(FT_Face) * FONT_CNT);
  return 0;
}

void
font_loader_destroy()
{
  for (int font_idx = 0; font_idx < FONT_CNT; ++font_idx)
  {
    for (int atlas_idx = 0; atlas_idx < FONT_ATLAS_SIZE; ++atlas_idx)
    {
      SDL_free(fontAtlas[font_idx][atlas_idx].texture);
    }
  }
}

static int generate_atlas(FT_Face    fontFace,
                          FontAtlas* atlas,
                          int        glyphMaxWidth,
                          int        glyphMaxHeight,
                          int        glyphCount);

int
font_loader_load(FontFace face, unsigned atlasSizeStart, unsigned atlasSizeEnd)
{
  FT_Error error    = 0;
  FT_Face  fontFace = faceList[face];
  if ((error = FT_New_Face(fontLibrary, fontDirs[face], 0, &fontFace)) != 0)
  {
    UZU_ERROR("Unable to load new face");
    return error;
  }

  for (unsigned pixelHeight = atlasSizeStart; pixelHeight <= atlasSizeEnd;
       ++pixelHeight)
  {
    // FT_Size_RequestRec size = { .type = FT_SIZE_REQUEST_TYPE_NOMINAL,
    //                            0,
    //                            75,
    //                            800*26,
    //                            600*26 };
    // if ((error = FT_Request_Size(fontFace, &size)) != 0)
    //  return error;
    if ((error = FT_Set_Pixel_Sizes(fontFace, 0, pixelHeight)) != 0)
      return error;
    int glyphMaxWidth  = 0;
    int glyphMaxHeight = 0;

    FT_GlyphSlot glyph = fontFace->glyph;
    for (int i = start_char; i <= end_char; ++i)
    {
      if ((error = FT_Load_Char(fontFace, i, FT_LOAD_RENDER)) != 0)
      {
        UZU_ERROR("Unable to load character \'%c\' in font face #%s",
                  (char)i,
                  fontDirs[face]);
        continue;
      }

      glyphMaxWidth  = max(glyphMaxWidth, glyph->bitmap.width);
      glyphMaxHeight = max(glyphMaxHeight, glyph->bitmap.rows);
    }
    FontAtlas* atlas   = &(fontAtlas[face][pixelHeight - atlasSizeStart]);
    atlas->pixelHeight = pixelHeight;
    error              = generate_atlas(fontFace,
                           atlas,
                           glyphMaxWidth,
                           glyphMaxHeight,
                           (end_char - start_char) + 1);
    if (error != 0)
    {
      return error;
    }
  }

  return 0;
}

const FontAtlas*
font_loader_get_atlas(FontFace face, unsigned atlasIndex)
{
  return &(fontAtlas[face][atlasIndex]);
}

static int
generate_atlas(FT_Face    fontFace,
               FontAtlas* atlas,
               int        glyphMaxWidth,
               int        glyphMaxHeight,
               int        glyphCount)
{
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

  for (int i = start_char; i < end_char; i++)
  {
    if (FT_Load_Char(fontFace, i, FT_LOAD_RENDER))
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

    atlas->charInfo[i].advance[0] =
        (float)((unsigned int)glyph->advance.x >> 6);
    atlas->charInfo[i].advance[1] =
        (float)((unsigned int)glyph->advance.y >> 6);
    atlas->charInfo[i].bitmapSize[0]    = (float)glyph->bitmap.width;
    atlas->charInfo[i].bitmapSize[1]    = (float)glyph->bitmap.rows;
    atlas->charInfo[i].bitmapBearing[0] = (float)glyph->bitmap_left;
    atlas->charInfo[i].bitmapBearing[1] = (float)glyph->bitmap_top;
    atlas->charInfo[i].texTopLeft[0]    = (float)offsetX / (float)atlas->width;
    atlas->charInfo[i].texTopLeft[1]    = (float)offsetY / (float)atlas->height;
    atlas->charInfo[i].texBottomRight[0] =
        (offsetX + (float)glyph->bitmap.width) / (float)atlas->width;
    atlas->charInfo[i].texBottomRight[1] =
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