#include "SDL_image.h"
#include "graphics.h"
#include "graphics/gl.h"

const static GLuint sPixelFormatToGLFormat[] = {
  [PIXEL_FORMAT_RGBA] = GL_RGBA,
  [PIXEL_FORMAT_BGRA] = GL_BGRA,
};

static unsigned int
get_surface_format(SDL_Surface* img)
{
  if (img->format->BytesPerPixel == 4)
  {
    if (img->format->Rmask == 0x000000ff)
      return PIXEL_FORMAT_RGBA;
    else
      return PIXEL_FORMAT_BGRA;
  }
  ASSERT_MSG(0, "unhandled pixel format");
  return 0;
}

int
texture_load(Texture* texture, const char* file)
{
  SDL_Surface* img = IMG_Load(file);
  if (!file || !texture)
  {
    UZU_ERROR("Invalid argument");
    return -1;
  }

  if (!img)
  {
    UZU_ERROR("Failed to read image file %s: %s", file, IMG_GetError());
    return -1;
  }

  PixelFormat format = get_surface_format(img);
  if (texture_load_from_memory(texture, img->pixels, img->w, img->h, format))
  {
    SDL_FreeSurface(img);
    return -1;
  }
  SDL_FreeSurface(img);
  return 0;
}

int
texture_load_from_memory(Texture* texture, const u8* data, int width, int height, PixelFormat dataFormat)
{
  glGenTextures(1, &texture->handle);
  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint glFormat = sPixelFormatToGLFormat[dataFormat];

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);

  texture->width  = width;
  texture->height = height;

  glBindTexture(GL_TEXTURE_2D, 0);
  return 0;
}

void
texture_bind(const Texture* texture)
{
  glBindTexture(GL_TEXTURE_2D, texture->handle);
}

void
texture_unbind(const Texture* texture)
{
  (void)texture;
  glBindTexture(GL_TEXTURE_2D, 0);
}

void
texture_destroy(Texture* texture)
{
  glDeleteTextures(1, &texture->handle);
}

void
texture_region_set_rect(TextureRegion* t, const IntRect* rect)
{
  if (rect == NULL)
  {
    t->rect.x = 0;
    t->rect.y = 0;
    t->rect.w = (int)t->texture->width;
    t->rect.h = (int)t->texture->height;
  }
  else
  {
    t->rect = *rect;
  }
  t->u1 = (float)t->rect.x / (float)t->texture->width;
  t->u2 = (float)(t->rect.x + t->rect.w) / (float)t->texture->width;
  t->v1 = (float)t->rect.y / (float)t->texture->height;
  t->v2 = (float)(t->rect.y + t->rect.h) / (float)t->texture->height;
}

void
texture_region_set_texture(TextureRegion* t, const Texture* texture, const IntRect* rect)
{
  t->texture = texture;
  texture_region_set_rect(t, rect);
}

TextureRegion
texture_region(const Texture* texture, const IntRect* rect)
{
  TextureRegion textureRegion;
  texture_region_set_texture(&textureRegion, texture, rect);
  return textureRegion;
}
