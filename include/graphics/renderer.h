// ngotrung Fri 10 Sep 2021 09:52:51 PM +07
#ifndef RENDERER_H
#define RENDERER_H
#include "graphics/material.h"
#include "graphics/types.h"

void draw(const Vertex*   vertcies,
          const Texture*  texture,
          const Material* material,
          int             vertex_count,
          int             sorting_layer,
          int             z_order);

#endif // RENDERER_H
