#include "components.h"

static const struct RelationshipComp s_dft_releationship = { .parent = ECS_NULL_ENT,
                                                             .next   = ECS_NULL_ENT,
                                                             .prev   = ECS_NULL_ENT,
                                                             .first  = ECS_NULL_ENT };

static const struct TransformComp s_dft_transform = { .scale = { 1.f, 1.f } };

static const struct SpriteComp s_dft_sprite = { .color = COLOR_WHITE_INIT };

#define TYPE_TRAITS(T, finiFn, cpyFn, dft)                                                                     \
    [T] = { .size          = sizeof(ECS_COMP_NM(T)),                                                           \
            .fini          = finiFn,                                                                           \
            .cpy           = cpyFn,                                                                            \
            .align         = _Alignof(ECS_COMP_NM(T)),                                                         \
            .default_value = dft,                                                                              \
            .name          = #T }
/* clang-format off */
/* component traits table */
const struct ecs_TypeTraits g_comp_traits[COMPONENT_CNT] = {
  TYPE_TRAITS(TransformComp, NULL, NULL, &s_dft_transform),
  TYPE_TRAITS(SpriteComp, NULL, NULL, &s_dft_sprite),
  TYPE_TRAITS(LocalTransformMatrixComp, NULL, NULL, NULL),
  TYPE_TRAITS(WorldTransformMatrixComp, NULL, NULL, NULL),
  TYPE_TRAITS(TransformChangedTag, NULL, NULL, NULL),
  TYPE_TRAITS(AnimationPoolComp, NULL, NULL, NULL),
  TYPE_TRAITS(DrawOrderComp, NULL, NULL, NULL),
  TYPE_TRAITS(VelocityComp, NULL, NULL, NULL),
  TYPE_TRAITS(MaterialComp, NULL, NULL, NULL),
  TYPE_TRAITS(AnimationComp, NULL, NULL, NULL),
  TYPE_TRAITS(ControllerComp, NULL, NULL, NULL),
  TYPE_TRAITS(InputComp, NULL, NULL, NULL),
  TYPE_TRAITS(SpeedComp, NULL, NULL, NULL),
  TYPE_TRAITS(CharacterAnimationControllerComp, NULL, NULL, NULL),
  TYPE_TRAITS(FacingDirectionComp, NULL, NULL, NULL),
  TYPE_TRAITS(HandComp, NULL, NULL, NULL),
  TYPE_TRAITS(HolderComp, NULL, NULL, NULL),
  TYPE_TRAITS(RelationshipComp, NULL, NULL, &s_dft_releationship),
  TYPE_TRAITS(HitBoxComp, NULL, NULL, NULL),
  TYPE_TRAITS(DestroyTag, NULL, NULL, NULL)
};
/* clang-format on */
