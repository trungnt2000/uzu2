#include "graphics/text.h"
#include "cglm/cglm.h"
#include "font_loader.h"
#include "graphics.h"
#include "text_format.h"
#include "toolbox.h"
#include <cglm/vec4.h>
#include <stdio.h>

/* default atlas */
static const FontAtlas* atlasDefault;

/* texture to use */
static Texture atlasDefaultTexture;

/* shader to use */
static TextShader defaultShader;

/* text format context */
static TextFormatContext fmtCxt;

/* wheather or not drawing */
static BOOL sIsDrawing;

/* EBO */
static GLuint sEbo;

/* VBO (position, uv, color)*/
static GLuint sVbo;

/* VAO */
static GLuint sVao;

/* vertex buffer*/
static TextVertex* sVertBuf;

/* next vertex pointer in vertex buffer */
static TextVertex* sNextVertPtr;

/* how many glyph are stored in buffer */
static u32 sGlyphCnt;

/* how many sprite we can batch together in one draw call */
static u32 sMaxGlyphs;

/* draw call count between begin and end call */
static u32 sDrawCallCnt;

static unsigned sScreenWidth;
static unsigned sScreenHeigth;

static void
upload_atlas_texture(Texture* tex, const FontAtlas* atlas)
{
  glGenTextures(1, &tex->handle);
  glBindTexture(GL_TEXTURE_2D, tex->handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RED,
               atlas->width,
               atlas->height,
               0,
               GL_RED,
               GL_UNSIGNED_BYTE,
               atlas->texture);

  tex->width  = atlas->width;
  tex->height = atlas->height;
  glBindTexture(GL_TEXTURE_2D, 0);
}

static inline void
text_indices_update(int glyphCount, unsigned* indices)
{
  for (int i = 0; i < glyphCount; ++i)
  {
    const int recOffset  = i * TEXT_IDX_PER_GLYPH;
    const int vertOffset = i * TEXT_VERT_PER_GLYPH;
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
text_verticies_update(const CharInfo* charInfo,
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

static inline void
prepare_graphic_buffers(TextVertex** vertices,
                        unsigned     maxGlyphs,
                        GLuint*      vertexArray,
                        GLuint*      vertexBuffer,
                        GLuint*      indexBuffer)
{
  const unsigned verticesCount = maxGlyphs * TEXT_VERT_PER_GLYPH;
  const unsigned vboSize       = sizeof(TextVertex) * verticesCount;
  *vertices                    = SDL_malloc(vboSize);

  const unsigned indicesCount = maxGlyphs * TEXT_IDX_PER_GLYPH;
  const unsigned eboSize      = sizeof(GLuint) * indicesCount;
  GLuint*        indices      = SDL_malloc(eboSize);

  text_indices_update(maxGlyphs, indices);

  glGenVertexArrays(1, vertexArray);
  glGenBuffers(1, vertexBuffer);
  glGenBuffers(1, indexBuffer);
  glBindVertexArray(*vertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize, indices, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, position));
  glVertexAttribPointer(1,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, color));
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(TextVertex),
                        (void*)offsetof(TextVertex, uv));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  SDL_free(indices);
}

int
text_renderer_init(u32              maxGlyphs,
                   const FontAtlas* atlas,
                   const vec4       defaultTextColor,
                   unsigned         screenWidth,
                   unsigned         screenHeigth)
{
  if (create_shader("res/shader/text.vert",
                    "res/shader/text.frag",
                    &defaultShader.handle) != 0)
  {
    UZU_ERROR("Failed to create program\n");
    return -1;
  }
  sMaxGlyphs    = maxGlyphs;
  atlasDefault  = atlas;
  sScreenWidth  = screenWidth;
  sScreenHeigth = screenHeigth;
  text_format_context_init(&fmtCxt, defaultTextColor);
  upload_atlas_texture(&atlasDefaultTexture, atlasDefault);
  prepare_graphic_buffers(&sVertBuf, sMaxGlyphs, &sVao, &sVbo, &sEbo);
  sIsDrawing = UZU_FALSE;

  const GLuint tex0Uni = glGetUniformLocation(defaultShader.handle, "tex0");
  glUseProgram(defaultShader.handle);
  glBindTexture(GL_TEXTURE_2D, atlasDefaultTexture.handle);
  glUniform1i(tex0Uni, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  return 0;
}

void
text_renderer_shutdown()
{
  glDeleteBuffers(1, &sEbo);
  glDeleteBuffers(1, &sVbo);
  glDeleteVertexArrays(1, &sVao);
  glDeleteTextures(1, &atlasDefaultTexture.handle);
  glDeleteProgram(defaultShader.handle);

  SDL_free(sVertBuf);
}

static void flush(void);

void
text_batch_begin()
{
  ASSERT_MSG(!sIsDrawing, "already drawing");
  sIsDrawing   = UZU_FALSE;
  sNextVertPtr = sVertBuf;
  sGlyphCnt    = 0;
  sDrawCallCnt = 0;
}

void
text_batch_end()
{
  // ASSERT_MSG(sIsDrawing, "call begin first!");
  flush();
  sIsDrawing = UZU_FALSE;
}

void
draw_text(const char* text, float x, float y, const vec4 color, float scale)
{
  const char* iter   = text;
  const float scaleW = 2.f / sScreenWidth;
  const float scaleH = 2.f / sScreenHeigth;

  int         codePointIdx = 0;
  TextVertex* vertGlyph;
  vertGlyph = sNextVertPtr;
  while (*iter)
  {
    vertGlyph = sNextVertPtr + ((long long)codePointIdx * TEXT_VERT_PER_GLYPH);
    text_verticies_update(&atlasDefault->charInfos[(u32)*iter],
                          vertGlyph,
                          color,
                          scale,
                          scaleW,
                          scaleH,
                          &x,
                          &y);
    ++codePointIdx;
    ++sGlyphCnt;
    ++iter;
  }
  sNextVertPtr = vertGlyph + 4;

  glUseProgram(defaultShader.handle);
  glBindTexture(GL_TEXTURE_2D, atlasDefaultTexture.handle);
}

static void
flush(void)
{
  GLsizei vboUpdtSiz;
  GLsizei numIndices;

  vboUpdtSiz = sizeof(TextVertex) * TEXT_VERT_PER_GLYPH * sGlyphCnt;
  numIndices = sGlyphCnt * TEXT_IDX_PER_GLYPH;

  /* update vertex buffer data */
  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboUpdtSiz, sVertBuf);

  glBindTexture(GL_TEXTURE_2D, atlasDefaultTexture.handle);

  /* draw all sprite in buffer */
  glBindVertexArray(sVao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  /* reset state */
  sGlyphCnt    = 0;
  sNextVertPtr = sVertBuf;
  sDrawCallCnt++;
}

// draw_text_ex(FontAtlas* font, char* text, float x, float y, vec4 color,
// TextAligment alignment) draw_text_boxed(FontAtlas* font, char* text, vec4
// box, TextWrapMode wrapMode, vec4 color) draw_text_boxed_ex(FontAtlas* font,
// char* text, vec4 box, TextWrapMode wrapMode, vec4 color, TextAlignment
// alignment)

void
text_update(TextFormatContext* ctx,
            const FontAtlas*   atlas,
            TextVertex*        vertices,
            unsigned*          indices,
            float              scale,
            int                screenWidth,
            int                screenHeigth,
            const vec2         position)
{
  float       xpos   = position[0];
  float       ypos   = position[1];
  const float scaleW = 2.f / screenWidth;
  const float scaleH = 2.f / screenHeigth;

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
      vertRec = vertices + ((long long)codePointIdx * TEXT_VERT_PER_GLYPH);
      text_verticies_update(&atlas->charInfos[ctx->codePoints[codePointIdx]],
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

  text_indices_update(ctx->codePointsSize, indices);
}