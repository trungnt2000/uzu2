#include "graphics/framebuffer.h"
#include "graphics/gl.h"

void
framebuffer_init(Framebuffer* framebuffer, int width, int height)
{
  glGenFramebuffers(1, &framebuffer->handle);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);

  // TODO: this block should be moved to some abstraction function like texture_create_ex(w, h,
  // internalformat)
  glGenTextures(1, &framebuffer->target.handle);
  glBindTexture(GL_TEXTURE_2D, framebuffer->target.handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
  framebuffer->target.width = width;
  framebuffer->target.height = height;

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->target.handle, 0);
  glDrawBuffers(1, (GLenum[]){ GL_COLOR_ATTACHMENT0 });

  glGenRenderbuffers(1, &framebuffer->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->rbo);

  ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
             "In completed framebuffer!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
framebuffer_destroy(Framebuffer* framebuffer)
{
  glDeleteFramebuffers(1, &framebuffer->handle);
  glDeleteTextures(1, &framebuffer->target.handle);
  glDeleteRenderbuffers(1, &framebuffer->rbo);
  framebuffer->handle        = 0;
  framebuffer->target.handle = 0;
  framebuffer->rbo           = 0;
}

void
framebuffer_bind(const Framebuffer* framebuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
}

void
framebuffer_unbind(const Framebuffer* framebuffer)
{
  (void)framebuffer;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
