#include "components.h"

#define TYPE_TRAITS(T, initFn, finiFn)                                         \
  [T] = { .size = sizeof(_##T),                                                \
          .init = (ecs_InitFunc)initFn,                                        \
          .fini = (ecs_FiniFunc)finiFn }
/* clang-format off */
/* component traits table */
const ecs_TypeTraits gCompTraits[] = {
  TYPE_TRAITS(Transform, NULL, NULL),
  TYPE_TRAITS(Sprite, NULL, NULL),
  TYPE_TRAITS(TransformMatrix, NULL, NULL),
  TYPE_TRAITS(TransformChanged, NULL, NULL),
  TYPE_TRAITS(AnimationPool, NULL, NULL)
};
/* clang-format on */
