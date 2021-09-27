#include "graphics.h"

const Sprite*
animation_get_frame(const Animation* anim, float elapsedTime)
{
  int index = (int)(elapsedTime / anim->frame_duration);
  index     = index % anim->frame_cnt;
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

  int numFrames       = tmpl->column_count * tmpl->row_count;
  int idx             = 0;
  anim->frames        = SDL_malloc(sizeof(*anim->frames) * numFrames);
  anim->frame_cnt      = numFrames;
  anim->frame_duration = tmpl->frame_duration;

  IntRect rect = {
    tmpl->offset_x,
    tmpl->offset_y,
    tmpl->sprite_width,
    tmpl->sprite_height,
  };
  for (int i = 0; i < tmpl->row_count; ++i)
  {
    for (int j = 0; j < tmpl->column_count; ++j)
    {
      sprite_set_texture(&anim->frames[idx++], texture, &rect);
      rect.x += tmpl->sprite_width;
    }
    rect.y += tmpl->sprite_height;
  }
}

void animation_init_w_sprite(Animation *animation, const Sprite *sprite, const AnimationTemplate *tmpl)
{
    AnimationTemplate tmpl_ = *tmpl;
    tmpl_.offset_x += sprite->rect.x;
    tmpl_.offset_y += sprite->rect.y;
    animation_init_w_texture(animation, sprite->texture, &tmpl_);
}

int animation_load_rwops(Animation *animation, SDL_RWops *stream)
{
  if (!animation || !stream)
  {
    UZU_ERROR("Invalid argument\n");
    return -1;
  }

  return 0;
}
