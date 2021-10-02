// ngotrung Fri 10 Sep 2021 09:52:51 PM +07
#ifndef RENDERER_H
#define RENDERER_H
#include "graphics/material.h"
#include "graphics/types.h"

enum DrawPrimitive
{
    DP_LINE,
    DP_TRIANGLE,
};

void begin_draw(void);

void end_draw(void);

void draw(const struct Vertex*   vertcies,
          enum DrawPrimitive     primitive,
          const struct Texture*  texture,
          const struct Material* material,
          u32                    vertex_count,
          mat4                   mvp_matrix,
          int                    priority);

#endif // RENDERER_H
