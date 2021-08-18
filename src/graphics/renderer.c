#include "cglm/cglm.h"
#include "graphics/gl.h"
#include "graphics/renderer.h"
#include "toolbox.h"

/* wheather or not drawing */
static bool sIsDrawing;

/* EBO */
static GLuint sEbo;

/* VBO (position, uv, color)*/
static GLuint sVbo;

/* VAO */
static GLuint sVao;

/* vertex buffer*/
static Vertex* sVertBuf;

/* next vertex pointer in vertex buffer */
static Vertex* sNextVertPtr;

/* how many sprite are stored in buffer */
static u32 sSpriteCnt;

/* how many sprite we can batch together in one draw call */
static u32 sMaxSprites;

/* draw call count between begin and end call */
static u32 sDrawCallCnt;

static u32 sVertCount;

static Texture sDefaultTexture;

static const Texture* sCurrentTexture;

static Font sDefaultFont;

static void submit(void);

void
sprite_renderer_init(u32 maxSprites)
{
  GLsizei vboSize, eboSize;

  sMaxSprites  = maxSprites;
  sSpriteCnt   = 0;
  sDrawCallCnt = 0;

  /* four vertcies per single sprite */
  vboSize = (GLsizei)(maxSprites * sizeof(Vertex) * 4);

  /* six indices per single sprite */
  eboSize = (GLsizei)(maxSprites * sizeof(u32) * 6);

  u32* indices = SDL_malloc((size_t)eboSize);
  for (u32 i = 0; i < maxSprites; ++i)
  {
    indices[(i * 6) + 0] = (i * 4) + 0;
    indices[(i * 6) + 1] = (i * 4) + 1;
    indices[(i * 6) + 2] = (i * 4) + 3;
    indices[(i * 6) + 3] = (i * 4) + 1;
    indices[(i * 6) + 4] = (i * 4) + 2;
    indices[(i * 6) + 5] = (i * 4) + 3;
  }

  glGenVertexArrays(1, &sVao);
  glGenBuffers(1, &sVbo);
  glGenBuffers(1, &sEbo);

  /*set up vao with one sVbo for position, texture coordinates, color and static
   * a ebo for indices */
  glBindVertexArray(sVao);

  /*pre-allocate memory for sVbo*/
  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize, indices, GL_STATIC_DRAW);

  /* store position attribute in vertex attribute list */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);

  /* store texture coordinates attribute in vertex attribute list */
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
  glEnableVertexAttribArray(1);

  /* store color attribute in vertex attribute list */
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  sCurrentTexture = NULL;
  sIsDrawing      = false;
  sVertBuf        = SDL_malloc((size_t)vboSize);

  SDL_free(indices);

  // load default shader, font, texture
  if (font_load(&sDefaultFont, "res/font/font.TTF", 16) != 0)
  {
    UZU_ERROR("Failed to load default font\n");
    return;
  }

  u32 white = 0xffffffff;
  if (texture_load_from_memory(&sDefaultTexture, (u8*)&white, 1, 1, PIXEL_FORMAT_RGBA) != 0)
  {
    UZU_ERROR("Failed to load default texture\n");
  }
}

void
sprite_renderer_shutdown()
{
  glDeleteBuffers(1, &sEbo);
  glDeleteBuffers(1, &sVbo);
  glDeleteVertexArrays(1, &sVao);
  SDL_free(sVertBuf);
}

void
draw_texture_region_w_tx(const TextureRegion* region,
                         vec2                 size,
                         vec2                 center,
                         vec4                 color,
                         float                depth,
                         mat3                 tx)
{
  ASSERT_MSG(sIsDrawing, "call begin first!");

  float          u1, v1, u2, v2; /* texture coordinates                          */
  Vertex*        vert;           /* vertex mem ptr                               */
  vec3           localPos;       /* vertex position in model space               */
  vec3           worldPos;       /* transformed position in world space          */
  bool           hasEnoughSpace; /* do we have enough space for one more sprite? */
  const Texture* texture;

  hasEnoughSpace = sSpriteCnt < sMaxSprites;

  /* if we do not have enough space for new one,
   * send all current draw commands to gpu */
  if (!hasEnoughSpace)
    submit();

  if (region == NULL || region->texture == NULL)
  {
    texture = &sDefaultTexture;
    u1      = 0.f;
    v1      = 0.f;
    v2      = 1.f;
    u2      = 1.f;
  }
  else
  {
    texture = region->texture;
    u1      = region->u1;
    v1      = region->v1;
    u2      = region->u2;
    v2      = region->v2;
  }

  /* if current texture to use diffrent from previous
   * texture flush current draw commands to gpu */
  if (sCurrentTexture != texture)
  {
    submit();
    sCurrentTexture = texture;
  }

  vert = sNextVertPtr;

  /* we need to transform four vertcies and then store them inside vertBuf */
  // top-left corner
  localPos[0] = 0.f - center[0];
  localPos[1] = 0.f - center[1];
  localPos[2] = 1.f;
  glm_mat3_mulv(tx, localPos, worldPos);

  vert->position[0] = worldPos[0];
  vert->position[1] = worldPos[1];
  vert->position[2] = depth;
  vert->texCoord[0] = u1;
  vert->texCoord[1] = v1;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // top-right corner
  localPos[0] = size[0] - center[0];
  localPos[1] = 0.f - center[1];
  localPos[2] = 1.f;
  glm_mat3_mulv(tx, localPos, worldPos);

  vert->position[0] = worldPos[0];
  vert->position[1] = worldPos[1];
  vert->position[2] = depth;
  vert->texCoord[0] = u2;
  vert->texCoord[1] = v1;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // bottom-right corner
  localPos[0] = size[0] - center[0];
  localPos[1] = size[1] - center[1];
  localPos[2] = 1.f;
  glm_mat3_mulv(tx, localPos, worldPos);

  vert->position[0] = worldPos[0];
  vert->position[1] = worldPos[1];
  vert->position[2] = depth;
  vert->texCoord[0] = u2;
  vert->texCoord[1] = v2;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // bottom-left corner
  localPos[0] = 0.f - center[0];
  localPos[1] = size[1] - center[1];
  localPos[2] = 1.f;
  glm_mat3_mulv(tx, localPos, worldPos);

  vert->position[0] = worldPos[0];
  vert->position[1] = worldPos[1];
  vert->position[2] = depth;
  vert->texCoord[0] = u1;
  vert->texCoord[1] = v2;
  glm_vec4_copy(color, vert->color);
  ++vert;

  sNextVertPtr = vert;
  sSpriteCnt++;
}

void
draw_texture_region(const TextureRegion* region, vec2 position, vec2 size, vec2 color)
{

  ASSERT_MSG(sIsDrawing, "call begin first!");

  float          u1, v1, u2, v2; /* texture coordinates                          */
  Vertex*        vert;           /* vertex mem ptr                               */
  SDL_bool       hasEnoughSpace; /* do we have enough space for one more sprite? */
  const Texture* texture;

  hasEnoughSpace = sSpriteCnt < sMaxSprites;

  /* if we do not have enough space for new one,
   * send all current draw commands to gpu */
  if (!hasEnoughSpace)
    submit();

  if (region == NULL || region->texture == NULL)
  {
    texture = &sDefaultTexture;
    u1      = 0.f;
    v1      = 0.f;
    v2      = 1.f;
    u2      = 1.f;
  }
  else
  {
    texture = region->texture;
    u1      = region->u1;
    v1      = region->v1;
    u2      = region->u2;
    v2      = region->v2;
  }
  /* if current texture to use diffrent from previous
   * texture flush current draw commands to gpu */
  if (sCurrentTexture != texture)
  {
    submit();
    sCurrentTexture = texture;
  }

  vert = sNextVertPtr;

  // top-left corner
  vert->position[0] = position[0];
  vert->position[1] = position[1];
  vert->position[2] = 0;
  vert->texCoord[0] = u1;
  vert->texCoord[1] = v1;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // top-right corner
  vert->position[0] = position[0] + size[0];
  vert->position[1] = position[1];
  vert->position[2] = 0;
  vert->texCoord[0] = u2;
  vert->texCoord[1] = v1;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // bottom-right corner
  vert->position[0] = position[0] + size[0];
  vert->position[1] = position[1] + size[1];
  vert->position[2] = 0;
  vert->texCoord[0] = u2;
  vert->texCoord[1] = v2;
  glm_vec4_copy(color, vert->color);
  ++vert;

  // bottom-left corner
  vert->position[0] = position[0];
  vert->position[1] = position[1] + size[1];
  vert->position[2] = 0;
  vert->texCoord[0] = u1;
  vert->texCoord[1] = v2;
  glm_vec4_copy(color, vert->color);
  ++vert;

  sNextVertPtr = vert;
  sSpriteCnt++;
}

static float
text_width(const char* text, const Font* font, float scale)
{
  float     textWidth = 0;
  const u8* iter      = (const u8*)text;
  u32       codepoint;
  while (*iter != '\0' && *iter != '\n')
  {
    codepoint = (u32)*iter++;
    textWidth += font->glyphs[codepoint].advance[0] * scale;
  }

  return textWidth;
}

void
draw_text(const char* text, float x, float y, vec4 color)
{
  draw_text_ex(text, &sDefaultFont, x, y, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_textv(const char* text, vec2 position, vec4 color)
{
  draw_textv_ex(text, &sDefaultFont, position, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_text_ex(const char*   text,
             const Font*   font,
             float         x,
             float         y,
             float         scale,
             TextAlignment alignment,
             vec4          color)
{
  draw_textv_ex(text, font, (vec2){ x, y }, scale, alignment, color);
}

static float
align_text(const char* text, const Font* font, float x, float scale, TextAlignment alignment)
{
  switch (alignment)
  {
  case TEXT_ALIGN_LEFT:
    return x;
    break;
  case TEXT_ALIGN_RIGHT:
    return x - text_width(text, font, scale);
    break;
  case TEXT_ALIGN_CENTER:
    return x - text_width(text, font, scale) / 2.f;
    break;
  }
}

static void draw_glyph(const Glyph* glyph, vec2 position, float scale, vec4 color);

static const char*
draw_single_line(const char* text, const Font* font, vec2 position, float scale, vec4 color)
{
  u32          codepoint;
  const u8*    iter    = (const u8*)text;
  vec2         drawPos = { position[0], position[1] + font->glyphMaxHeight * scale };
  const Glyph* glyph;

  while (*iter && *iter != '\n')
  {
    codepoint = (u32)*iter++;
    glyph     = &font->glyphs[codepoint];
    draw_glyph(glyph, drawPos, scale, color);
    drawPos[0] += glyph->advance[0] * scale;
  }

  return (const char*)(*iter == '\0' ? NULL : iter + 1);
}

void
draw_textv_ex(const char*   text,
              const Font*   font,
              vec2          position,
              float         scale,
              TextAlignment alignment,
              vec4          color)
{
  // TODO: decode utf8
  const char* line = text;
  vec2        drawPos;

  if (font == NULL)
    font = &sDefaultFont;

  drawPos[1] = position[1];

  while (line != NULL)
  {
    drawPos[0] = align_text(line, font, position[0], scale, alignment);
    line       = draw_single_line(line, font, drawPos, scale, color);
    drawPos[1] += font->glyphMaxHeight * scale;
  }
}

void draw_text_boxed(const char* text, vec4 box, vec4 color);

bool
get_next_line(const char** textPtr,
              const Font*  font,
              u8*          buf,
              int          bufsize,
              float        widthLimit,
              float*       widthReturn)
{
  float     width = 0;
  int       count = 0;
  const u8* text  = (const u8*)*textPtr;
  if (text == NULL)
    return false;

  while (*text && *text != '\n' && width < widthLimit && count < bufsize - 1)
  {
    width += font->glyphs[(u32)*text].advance[0];
    *buf++ = *text++;
    ++count;
  }
  *buf         = '\0';
  *widthReturn = width;
  *textPtr     = (const char*)(*text == '\0' ? NULL : text + 1);

  return count != 0;
}

void
draw_text_boxed_ex(const char*   text,
                   const Font*   font,
                   vec2          position,
                   vec2          size,
                   float         scale,
                   TextAlignment alignment,
                   TextWrap      wrap,
                   vec4          color)
{
  u8           buf[512];
  float        lineWidth;
  u8*          iter;
  const Glyph* glyph;
  const float  rowSize = font->glyphMaxHeight * scale;
  vec2         drawPos = { [1] = position[1] + rowSize };

  if (font == NULL)
    font = &sDefaultFont;

  while (get_next_line(&text, font, buf, 512, size[1] / scale, &lineWidth))
  {
    iter       = buf;
    drawPos[0] = align_text((const char*)iter, font, position[0], scale, alignment);
    while (*iter)
    {
      glyph = &font->glyphs[(u32)*iter++];
      draw_glyph(glyph, drawPos, scale, color);
      drawPos[0] += glyph->advance[0] * scale;
    }
    drawPos[1] += rowSize;
  }
}

static void
draw_glyph(const Glyph* glyph, vec2 position, float scale, vec4 color)
{
  vec2 drawPos;
  vec2 size;

  size[0] = glyph->size[0] * scale;
  size[1] = glyph->size[1] * scale;

  drawPos[0] = position[0] + glyph->bearing[0] * scale;
  drawPos[1] = position[1] - glyph->bearing[1] * scale;
  draw_texture_region(&glyph->tex, drawPos, size, color);
}

void
draw_codepoint(u32 codepoint, const Font* font, vec2 position, float scale, vec4 color)
{
  const Glyph* glyph = &font->glyphs[codepoint];
  draw_glyph(glyph, position, scale, color);
}

void
sprite_batch_begin()
{
  ASSERT_MSG(!sIsDrawing, "already drawing");
  sIsDrawing      = true;
  sNextVertPtr    = sVertBuf;
  sCurrentTexture = NULL;
  sSpriteCnt      = 0;
  sDrawCallCnt    = 0;
  sVertCount      = 0;
  sCurrentTexture = &sDefaultTexture;
}

void
sprite_batch_end()
{
  ASSERT_MSG(sIsDrawing, "call begin first!");
  submit();
  sIsDrawing = false;
}

void
sprite_renderer_query_statistics(RenderStatistics* statistics)
{
  statistics->drawCall     = sDrawCallCnt;
  statistics->verticeCount = sVertCount;
}

static void
submit(void)
{
  GLsizei vboUpdtSiz;
  GLsizei numIndices;

  vboUpdtSiz = sizeof(Vertex) * 4 * sSpriteCnt;
  numIndices = sSpriteCnt * 6;

  /* update vertex buffer data */
  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboUpdtSiz, sVertBuf);

  texture_bind(sCurrentTexture);

  /* draw all sprite in buffer */
  glBindVertexArray(sVao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* reset state */
  sSpriteCnt   = 0;
  sNextVertPtr = sVertBuf;
  sDrawCallCnt++;
  sVertCount += numIndices;
}
