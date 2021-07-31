#include "cglm/cglm.h"
#include "graphics.h"
#include "toolbox.h"

/* structure keep track rendering state and all releated gl object*/
struct SpriteRS
{
  /* texture to use */
  const Texture* texture;

  /* wheather or not drawing */
  BOOL isDrawing;

  /* EBO */
  GLuint ebo;

  /* VBO (position, uv, color)*/
  GLuint vbo;

  /* VAO */
  GLuint vao;

  /* vertex buffer*/
  Vertex* vertBuf;

  /* next vertex pointer in vertex buffer */
  Vertex* nextVertPtr;

  /* how many sprite are stored in buffer */
  u32 spriteCnt;

  /* how many sprite we can batch together in one draw call */
  u32 maxSprites;

  /* draw call count between begin and end call */
  u32 drawCallCnt;
};

static void
flush(SpriteRS* state)
{
  GLsizei vboUpdtSiz;
  GLsizei numIndices;

  vboUpdtSiz = sizeof(Vertex) * 4 * state->spriteCnt;
  numIndices = state->spriteCnt * 6;

  /* update vertex buffer data */
  glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboUpdtSiz, state->vertBuf);

  glBindTexture(GL_TEXTURE_2D, state->texture->handle);

  /* draw all sprite in buffer */
  glBindVertexArray(state->vao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  /* reset state */
  state->spriteCnt   = 0;
  state->nextVertPtr = state->vertBuf;
  state->drawCallCnt++;
}

void
draw_sprite(SpriteRS*            state,
            vec2                 size,
            vec2                 center,
            vec4                 color,
            float                depth,
            const TextureRegion* textureRegion,
            mat3                 transMat)
{
  ASSERT_MSG(state->isDrawing, "call begin first!");

  float    u1, v1, u2, v2; /* texture coordinates                          */
  Vertex*  vert;           /* vertex mem ptr                               */
  vec3     localPos;       /* vertex position in model space               */
  vec3     worldPos;       /* transformed position in world space          */
  SDL_bool hasEnoughSpace; /* do we have enough space for one more sprite? */

  hasEnoughSpace = state->spriteCnt < state->maxSprites;

  /* if we do not have enough space for new one,
   * send all current draw commands to gpu */
  if (!hasEnoughSpace)
    flush(state);

  /* if current texture to use diffrent from previous
   * texture flush current draw commands to gpu */
  if (state->texture != textureRegion->texture)
  {
    if (state->texture != NULL)
    {
      flush(state);
    }
    state->texture = textureRegion->texture;
  }

  vert = state->nextVertPtr;

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

  state->nextVertPtr = vert;
  state->spriteCnt++;
}

SpriteRS*
sprite_render_state_create(u32 maxSprites)
{
  GLsizei   vboSize, eboSize;
  SpriteRS* state = SDL_malloc(sizeof(SpriteRS));

  state->maxSprites  = maxSprites;
  state->spriteCnt   = 0;
  state->drawCallCnt = 0;

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

  glGenVertexArrays(1, &state->vao);
  glGenBuffers(1, &state->vbo);
  glGenBuffers(1, &state->ebo);

  /*set up vao with one vbo for position, texture coordinates, color and static
   * a ebo for indices */
  glBindVertexArray(state->vao);

  /*pre-allocate memory for vbo*/
  glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
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

  state->texture   = NULL;
  state->isDrawing = FALSE;
  state->vertBuf   = SDL_malloc(vboSize);

  SDL_free(indices);
  return state;
}

void
sprite_render_state_destroy(SpriteRS* state)
{
  if (state)
  {
    glDeleteBuffers(1, &state->ebo);
    glDeleteBuffers(1, &state->vbo);
    glDeleteVertexArrays(1, &state->vao);
    SDL_free(state->vertBuf);
    SDL_free(state);
  }
}

void
begin_draw_sprite(SpriteRS* state)
{
  ASSERT_MSG(!state->isDrawing, "already drawing");
  state->isDrawing   = TRUE;
  state->nextVertPtr = state->vertBuf;
  state->texture     = NULL;
  state->spriteCnt   = 0;
  state->drawCallCnt = 0;
}

void
end_draw_sprite(SpriteRS* state)
{
  ASSERT_MSG(state->isDrawing, "call begin first!");
  flush(state);
  state->isDrawing = FALSE;
}
