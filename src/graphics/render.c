#include "cglm/cglm.h"
#include "graphics.h"
#include "toolbox.h"

/* texture to use */
static const Texture* sTexture;

/* wheather or not drawing */
static BOOL sIsDrawing;

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

static void
flush(void)
{
  GLsizei vboUpdtSiz;
  GLsizei numIndices;

  vboUpdtSiz = sizeof(Vertex) * 4 * sSpriteCnt;
  numIndices = sSpriteCnt * 6;

  /* update vertex buffer data */
  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboUpdtSiz, sVertBuf);

  glBindTexture(GL_TEXTURE_2D, sTexture->handle);

  /* draw all sprite in buffer */
  glBindVertexArray(sVao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  /* reset state */
  sSpriteCnt   = 0;
  sNextVertPtr = sVertBuf;
  sDrawCallCnt++;
}

void
draw_sprite(vec2                 size,
            vec2                 center,
            vec4                 color,
            float                depth,
            const TextureRegion* textureRegion,
            mat3                 transMat)
{
  ASSERT_MSG(sIsDrawing, "call begin first!");

  float    u1, v1, u2, v2; /* texture coordinates                          */
  Vertex*  vert;           /* vertex mem ptr                               */
  vec3     localPos;       /* vertex position in model space               */
  vec3     worldPos;       /* transformed position in world space          */
  SDL_bool hasEnoughSpace; /* do we have enough space for one more sprite? */

  hasEnoughSpace = sSpriteCnt < sMaxSprites;

  /* if we do not have enough space for new one,
   * send all current draw commands to gpu */
  if (!hasEnoughSpace)
    flush();

  /* if current texture to use diffrent from previous
   * texture flush current draw commands to gpu */
  if (sTexture != textureRegion->texture)
  {
    if (sTexture != NULL)
    {
      flush();
    }
    sTexture = textureRegion->texture;
  }

  vert = sNextVertPtr;

  u1 = textureRegion->u1;
  v1 = textureRegion->v1;
  u2 = textureRegion->u2;
  v2 = textureRegion->v2;

  /* we need to transform four vertcies and then store them inside vertBuf */
  // top-left corner
  localPos[0] = 0.f - center[0];
  localPos[1] = 0.f - center[1];
  localPos[2] = 1.f;
  glm_mat3_mulv(transMat, localPos, worldPos);

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
  glm_mat3_mulv(transMat, localPos, worldPos);

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
  glm_mat3_mulv(transMat, localPos, worldPos);

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
  glm_mat3_mulv(transMat, localPos, worldPos);

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
sprite_renderer_init(u32 maxSprites)
{
  GLsizei vboSize, eboSize;

  sMaxSprites  = maxSprites;
  sSpriteCnt   = 0;
  sDrawCallCnt = 0;

  /* four vertcies per single sprite */
  vboSize = maxSprites * sizeof(Vertex) * 4;

  /* six indices per single sprite */
  eboSize = maxSprites * sizeof(u32) * 6;

  u32* indices = SDL_malloc(sizeof(u32) * maxSprites * 6);
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
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Vertex),
                        (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);

  /* store texture coordinates attribute in vertex attribute list */
  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Vertex),
                        (void*)offsetof(Vertex, texCoord));
  glEnableVertexAttribArray(1);

  /* store color attribute in vertex attribute list */
  glVertexAttribPointer(2,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Vertex),
                        (void*)offsetof(Vertex, color));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  sTexture   = NULL;
  sIsDrawing = FALSE;
  sVertBuf   = SDL_malloc(vboSize);

  SDL_free(indices);
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
sprite_batch_begin()
{
  ASSERT_MSG(!sIsDrawing, "already drawing");
  sIsDrawing   = TRUE;
  sNextVertPtr = sVertBuf;
  sTexture     = NULL;
  sSpriteCnt   = 0;
  sDrawCallCnt = 0;
}

void
sprite_batch_end()
{
  ASSERT_MSG(sIsDrawing, "call begin first!");
  flush();
  sIsDrawing = FALSE;
}
