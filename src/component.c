#include "components.h"

#define TYPE_TRAITS(T, finiFn, cpyFn)                                          \
  [T] = { .size  = sizeof(ECS_COMP_NM(T)),                                     \
          .fini  = (ecs_FiniFunc)finiFn,                                       \
          .cpy   = cpyFn,                                                      \
          .align = _Alignof(ECS_COMP_NM(T)) }
/* clang-format off */
/* component traits table */
const ecs_TypeTraits gCompTraits[COMPONENT_CNT] = {
  TYPE_TRAITS(Transform, NULL, NULL),
  TYPE_TRAITS(Sprite, NULL, NULL),
  TYPE_TRAITS(TransformMatrix, NULL, NULL),
  TYPE_TRAITS(TransformChanged, NULL, NULL),
  TYPE_TRAITS(AnimationPool, NULL, NULL),
  TYPE_TRAITS(DrawOrder, NULL, NULL),
  TYPE_TRAITS(Text, NULL, NULL)
};
/* clang-format on */
