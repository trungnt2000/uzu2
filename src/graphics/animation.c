#include "graphics.h"

const TextureRegion*
animation_get_frame(const Animation* anim, float elapsedTime)
{
  int index = (int)(elapsedTime / anim->frameDuration);
  index     = index % anim->frameCnt;
  return &anim->frames[index];
}

void
animation_destroy(Animation* anim)
{
  SDL_free(anim->frames);
  anim->frames = NULL;
}

void animation_init_w_texture(Animation *anim, const Texture *texture, const AnimationTemplate *tmpl)
{

  int numFrames       = tmpl->columnCount * tmpl->rowCount;
  int idx             = 0;
  anim->frames        = SDL_malloc(sizeof(*anim->frames) * numFrames);
  anim->frameCnt      = numFrames;
  anim->frameDuration = tmpl->frameDuration;

  IntRect rect = {
    tmpl->xOffset,
    tmpl->yOffset,
    tmpl->spriteWidth,
    tmpl->spriteHeight,
  };
  for (int i = 0; i < tmpl->rowCount; ++i)
  {
    for (int j = 0; j < tmpl->columnCount; ++j)
    {
      texture_region_set_texture(&anim->frames[idx++], texture, &rect);
      rect.x += tmpl->spriteWidth;
    }
    rect.y += tmpl->spriteHeight;
  }
}

void animation_init_w_sprite(Animation *animation, const TextureRegion *sprite, const AnimationTemplate *tmpl)
{
    AnimationTemplate tmpl_ = *tmpl;
    tmpl_.xOffset += sprite->rect.x;
    tmpl_.yOffset += sprite->rect.y;
    animation_init_w_texture(animation, sprite->texture, &tmpl_);
}
