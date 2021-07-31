#include "graphics.h"

void
anim_init_tmpl(Animation* anim, const AnimationTemplate* tmpl)
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
      texture_region_set_texture(&anim->frames[idx++], tmpl->texture, &rect);
      rect.x += tmpl->spriteWidth;
    }
    rect.y += tmpl->spriteHeight;
  }
}

const TextureRegion*
anim_get_frame(const Animation* anim, float elapsedTime)
{
  int index = (int)(elapsedTime / anim->frameDuration);
  index     = index % anim->frameCnt;
  return &anim->frames[index];
}

void
anim_destroy(Animation* anim)
{
  SDL_free(anim->frames);
  anim->frames = NULL;
}
