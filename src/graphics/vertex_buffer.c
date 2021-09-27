#include "graphics/vertex_buffer.h"
#include "graphics/gl.h"

VertexBuffer*
vertex_buffer_init(VertexBuffer* buf, u32 size)
{
  u32* indices;
  buf->size  = size;
  buf->count = 0;

  /* four vertcies per single quad */
  const size_t vboSize = (size * sizeof(Vertex) * 4);

  /* six indices per single quad  */
  const size_t eboSize = (size * sizeof(*indices) * 6);

  indices       = SDL_malloc((size_t)eboSize);
  buf->vertices = SDL_malloc(sizeof(*buf->vertices) * size);
  for (u32 i = 0; i < size; ++i)
  {
    indices[(i * 6) + 0] = (i * 4) + 0;
    indices[(i * 6) + 1] = (i * 4) + 1;
    indices[(i * 6) + 2] = (i * 4) + 3;
    indices[(i * 6) + 3] = (i * 4) + 1;
    indices[(i * 6) + 4] = (i * 4) + 2;
    indices[(i * 6) + 5] = (i * 4) + 3;
  }

  glGenVertexArrays(1, &buf->vao);
  glGenBuffers(1, &buf->vbo);
  glGenBuffers(1, &buf->ebo);

  /*set up vao with one sVbo for position, texture coordinates, color and static
   * a ebo for indices */
  glBindVertexArray(buf->vao);

  /*pre-allocate memory for VBO */
  glBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vboSize, NULL, GL_DYNAMIC_DRAW);

  /* upload indices data to EBO */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)eboSize, indices, GL_STATIC_DRAW);
  SDL_free(indices);

  /* store position attribute in vertex attribute list */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);

  /* store texture coordinates attribute in vertex attribute list */
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
  glEnableVertexAttribArray(1);

  /* store color attribute in vertex attribute list */
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  return buf;
}

void
vertex_buffer_destroy(VertexBuffer* buf)
{
  if (buf == NULL)
    return;
  glDeleteBuffers(1, &buf->vbo);
  glDeleteBuffers(1, &buf->ebo);
  glDeleteVertexArrays(1, &buf->vao);
  SDL_free(buf->vertices);

  buf->ebo      = 0;
  buf->vao      = 0;
  buf->vbo      = 0;
  buf->vertices = NULL;
}

void
vertex_buffer_push_vertex(VertexBuffer* buf, const Vertex* vertex)
{
  buf->vertices[buf->count++] = *vertex;
}

void
vertex_buffer_push_nvertex(VertexBuffer* buf, const Vertex* vertiecs, u32 n)
{
  SDL_memcpy(&buf->vertices[buf->count], vertiecs, (sizeof *vertiecs) * n);
  buf->count += 4;
}

u32
vertex_buffer_availiable(const VertexBuffer* buf)
{
  return buf->size - buf->count;
}

void
draw_vertex_buffer(VertexBuffer* buf)
{
  GLsizei vboUpdtSiz;
  GLsizei numIndices;

  vboUpdtSiz = (GLsizei)(buf->count * sizeof(Vertex));
  numIndices = (GLsizei)((buf->count / 4) * 6);

  /* update vertex buffer data */
  glBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboUpdtSiz, buf->vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(buf->vao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  vertex_buffer_rewind(buf);
}

void
vertex_buffer_rewind(VertexBuffer* vertBuf)
{
  vertBuf->count = 0;
}
