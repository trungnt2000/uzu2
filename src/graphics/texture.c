#include "SDL_image.h"
#include "graphics.h"

static unsigned int
get_gl_pixel_format(SDL_Surface* img)
{
  if (img->format->BytesPerPixel == 4)
  {
    if (img->format->Rmask == 0x000000ff)
      return GL_RGBA;
    else
      return GL_BGRA;
  }
  else
  {
    if (img->format->Rmask == 0x000000ff)
      return GL_RGB;
    else
      return GL_BGR;
  }
  // ASSERT_MSG(0, "unhandled pixel format");
  // return 0;
}

int
texture_load(Texture* texture, const char* file)
{
  SDL_Surface* img = IMG_Load(file);
  if (!file || !texture)
  {
    UZU_ERROR("Invalid Argument");
    return -1;
  }
  if (!img)
  {
    UZU_ERROR("Failed to read image file %s: %s", file, IMG_GetError());
    return -1;
  }

  glGenTextures(1, &texture->handle);
  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               img->w,
               img->h,
               0,
               get_gl_pixel_format(img),
               GL_UNSIGNED_BYTE,
               img->pixels);

  texture->width  = img->w;
  texture->height = img->h;

  glBindTexture(GL_TEXTURE_2D, 0);
  SDL_FreeSurface(img);
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
texture_free(Texture* texture)
{
  glDeleteTextures(1, &texture->handle);
}

void
texture_region_set_rect(TextureRegion* t, const IntRect* rect)
{
  if (rect == NULL)
  {
    t->texRect.x = 0;
    t->texRect.y = 0;
    t->texRect.w = t->texture->width;
    t->texRect.h = t->texture->height;
  }
  else
  {
    t->texRect = *rect;
  }
  t->u1 = (float)t->texRect.x / (float)t->texture->width;
  t->u2 = (float)(t->texRect.x + t->texRect.w) / (float)t->texture->width;
  t->v1 = (float)t->texRect.y / (float)t->texture->height;
  t->v2 = (float)(t->texRect.y + t->texRect.h) / (float)t->texture->height;
}

void
texture_region_set_texture(TextureRegion* t,
                           const Texture* texture,
                           const IntRect* rect)
{
  t->texture = texture;
  texture_region_set_rect(t, rect);
}
