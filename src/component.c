#include "components.h"

#define TYPE_TRAITS(T, finiFn, cpyFn)                                                                          \
    [T] = { .size  = sizeof(ECS_COMP_NM(T)),                                                                   \
            .fini  = finiFn,                                                                                   \
            .cpy   = cpyFn,                                                                                    \
            .align = _Alignof(ECS_COMP_NM(T)),                                                                 \
            .name  = #T }
/* clang-format off */
/* component traits table */
const struct ecs_TypeTraits g_comp_traits[COMPONENT_CNT] = {
  TYPE_TRAITS(TransformComp, NULL, NULL),
  TYPE_TRAITS(SpriteComp, NULL, NULL),
  TYPE_TRAITS(LocalTransformMatrixComp, NULL, NULL),
  TYPE_TRAITS(WorldTransformMatrixComp, NULL, NULL),
  TYPE_TRAITS(TransformChangedTag, NULL, NULL),
  TYPE_TRAITS(AnimationPoolComp, NULL, NULL),
  TYPE_TRAITS(DrawOrderComp, NULL, NULL),
  TYPE_TRAITS(VelocityComp, NULL, NULL),
  TYPE_TRAITS(MaterialComp, NULL, NULL),
  TYPE_TRAITS(AnimationComp, NULL, NULL),
  TYPE_TRAITS(ControllerComp, NULL, NULL),
  TYPE_TRAITS(InputComp, NULL, NULL),
  TYPE_TRAITS(SpeedComp, NULL, NULL),
  TYPE_TRAITS(CharacterAnimationControllerComp, NULL, NULL),
  TYPE_TRAITS(FacingDirectionComp, NULL, NULL),
  TYPE_TRAITS(HolderComp, NULL, NULL),
  TYPE_TRAITS(RelationshipComp, NULL, NULL),
  TYPE_TRAITS(HitBoxComp, NULL, NULL),
  TYPE_TRAITS(DestroyTag, NULL, NULL),
  TYPE_TRAITS(NameComp, NULL, NULL),
  TYPE_TRAITS(WorldTransformMatrixChangedTag, NULL, NULL),
  TYPE_TRAITS(EntityTag, NULL, NULL),
  TYPE_TRAITS(RotatorComp, NULL, NULL),
  TYPE_TRAITS(InteractableComp, NULL, NULL),
  TYPE_TRAITS(RefComp, NULL, NULL),
  TYPE_TRAITS(AffineAnimComp, NULL, NULL),
  TYPE_TRAITS(SwingWeaponComp, NULL, NULL),


  TYPE_TRAITS(DefenseComp, NULL, NULL),
  TYPE_TRAITS(MagicalAtkPowComp, NULL, NULL),
  TYPE_TRAITS(PhysicalAtkPowerComp, NULL, NULL),
  TYPE_TRAITS(HealthPoolComp, NULL, NULL),
  TYPE_TRAITS(ManaPoolComp, NULL, NULL),


  TYPE_TRAITS(BaseStatsComp, NULL, NULL),
  TYPE_TRAITS(BaseStatsChangedTag, NULL, NULL),
  TYPE_TRAITS(StatModifiersComp, NULL, NULL),
  TYPE_TRAITS(StatModifiersChangedTag, NULL, NULL),
  TYPE_TRAITS(EquipmentChangedTag, NULL, NULL),
  TYPE_TRAITS(EquipmentSlotsComp, NULL, NULL)
};
/* clang-format on */
