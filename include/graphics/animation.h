// ngotrung Wed 18 Aug 2021 11:23:48 PM +07
#ifndef GRAPHICS_ANIMATION_H
#define GRAPHICS_ANIMATION_H
#include "graphics/sprite_sheet.h"
#include "graphics/types.h"

typedef struct Animation
{
  Sprite* frames;
  int     frame_cnt;
  float   frame_duration;
} Animation;

typedef struct AnimationTemplate
{
  int   offset_x;
  int   offset_y;
  int   sprite_width;
  int   sprite_height;
  int   column_count;
  int   row_count;
  float frame_duration;
} AnimationTemplate;

void animation_init_w_texture(Animation* animation, const Texture* texture, const AnimationTemplate* tmpl);

void animation_init_w_sprite(Animation* animation, const Sprite* sprite, const AnimationTemplate* tmpl);

void animation_destroy(Animation* animation);

const Sprite* animation_get_frame(const Animation* animation, float elapsed_time);

int animation_load(Animation* animation, const char* file);

int animation_load_rwops(Animation* animation, SDL_RWops* stream);

#endif // GRAPHICS_ANIMATION_H
