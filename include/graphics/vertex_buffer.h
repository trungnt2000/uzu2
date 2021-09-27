// ngotrung Sun 22 Aug 2021 01:53:26 PM +07
#ifndef GRAPHICS_VERTEX_BUFFER_H
#define GRAPHICS_VERTEX_BUFFER_H

#include "graphics/types.h"

typedef struct VertexBuffer
{
  Vertex*          vertices;
  u32              count;
  u32              size;
  u32              vao;
  u32              vbo;
  u32              ebo;
} VertexBuffer;

VertexBuffer* vertex_buffer_init(VertexBuffer* vbuf, u32 size);

void vertex_buffer_destroy(VertexBuffer* vbuf);

void vertex_buffer_push_vertex(VertexBuffer* vbuf, const Vertex* vertex);

void vertex_buffer_push_nvertex(VertexBuffer* vbuf, const Vertex* vertiecs, u32 n);

void vertex_buffer_rewind(VertexBuffer* vbuf);

u32 vertex_buffer_availiable(const VertexBuffer* vbuf);

void draw_vertex_buffer(VertexBuffer* vbuf);

#endif // GRAPHICS_VERTEX_BUFFER_H
