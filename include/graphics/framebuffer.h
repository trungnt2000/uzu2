// ngotrung Sat 21 Aug 2021 12:32:11 PM +07
#ifndef GRAPHICS_FRAME_BUFFER_H
#define GRAPHICS_FRAME_BUFFER_H
#include "graphics/types.h"

typedef struct Framebuffer
{
  unsigned int handle;
  Texture      target;
  unsigned int rbo;
} Framebuffer;

void framebuffer_init(Framebuffer* framebuffer, int width, int height);
void framebuffer_destroy(Framebuffer* framebuffer);
void framebuffer_bind(const Framebuffer* framebuffer);
void framebuffer_unbind(const Framebuffer* framebuffer);
#endif // GRAPHICS_FRAME_BUFFER_H
