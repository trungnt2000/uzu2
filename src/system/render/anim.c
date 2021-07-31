// ngotrung Wed 28 Jul 2021 10:49:48 PM +07
#ifndef SYSTEM_RENDER_ANIM_H
#define SYSTEM_RENDER_ANIM_H
#include "components.h"
#include "ecs.h"

typedef struct Dependencies
{
  _Sprite*        sprite;
  _AnimationPool* animPool;
} Dependencies;

void
system_rendering_animation_update(ecs_Registry* registry, float deltaTime)
{
  ecs_View             view;
  ecs_entity_t         ett;
  Dependencies         deps;
  const TextureRegion* keyframe;

  ecs_view_init(&view, registry, { Sprite, AnimationPool });
  while (ecs_view_next(&view, &ett, &deps))
  {
    deps.animPool->elapsedTime += deltaTime;
    keyframe = anim_get_frame(deps.animPool->anims, deps.animPool->elapsedTime);
    deps.sprite->textureRegion = *keyframe;
  }
}
#endif // SYSTEM_RENDER_ANIM_H
