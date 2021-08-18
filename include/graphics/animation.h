// ngotrung Wed 18 Aug 2021 11:23:48 PM +07
#ifndef GRAPHICS_ANIMATION_H
#define GRAPHICS_ANIMATION_H
#include "graphics/sprite_sheet.h"
#include "graphics/types.h"

typedef struct Animation
{
  TextureRegion* frames;
  int            frameCnt;
  float          frameDuration;
} Animation;

typedef struct AnimationTemplate
{
  int            xOffset;
  int            yOffset;
  int            spriteWidth;
  int            spriteHeight;
  int            columnCount;
  int            rowCount;
  float          frameDuration;
} AnimationTemplate;

void animation_init_w_texture(Animation* animation, const Texture* texture, const AnimationTemplate* tmpl);

void
animation_init_w_sprite(Animation* animation, const TextureRegion* sprite, const AnimationTemplate* tmpl);

void                 animation_destroy(Animation* animation);
const TextureRegion* animation_get_frame(const Animation* animation, float elapsedTime);


#endif // GRAPHICS_ANIMATION_H
