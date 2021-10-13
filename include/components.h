// ngotrung Sat 24 Jul 2021 11:58:27 PM +07
#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "cglm/cglm.h"
#include "ecs.h"
#include "equipment_system.h"
#include "global.h"
#include "graphics.h"
#include "toolbox/common.h"

enum ComponentId
{
    TransformComp,
    SpriteComp,
    TransformChangedTag,
    AnimationPoolComp,
    DrawOrderComp,
    VelocityComp,
    MaterialComp,
    AnimationComp,
    InputComp,
    ControllerComp,
    CharacterAnimationControllerComp,
    FacingDirectionComp,
    HolderComp,
    LocalTransformMatrixComp,
    WorldTransformMatrixComp,
    RelationshipComp,
    HitBoxComp,
    DestroyTag,
    NameComp,
    WorldTransformMatrixChangedTag,
    EntityTag,
    RotatorComp,
    InteractableComp,
    RefComp,
    AffineAnimComp,
    SwingWeaponComp,
    /* Stats components */
    SpeedComp,
    DefenseComp,
    MagicalAtkPowComp,
    PhysicalAtkPowerComp,
    HealthPoolComp,
    ManaPoolComp,

    BaseStatsComp,
    BaseStatsChangedTag,
    StatModifiersComp,
    StatModifiersChangedTag,

    EquipmentChangedTag,
    EquipmentSlotsComp,
    /**/
    COMPONENT_CNT
};

extern const struct ecs_TypeTraits g_comp_traits[];

struct TransformComp
{
    vec3  position;
    vec2  scale;
    float rotation;
};

#define TRANSFORM_COMP_INIT                                                                                    \
    {                                                                                                          \
        .position = { 0.f, 0.f }, .scale = { 1.f, 1.f }, .rotation = 0.f                                       \
    }

/* this tag component use in case some thing have changed
 * our transform component */
struct TransformChangedTag
{
    int dummy;
};

struct DestroyTag
{
    int dummy;
};

struct SpriteComp
{
    vec4      color;
    Material* material;
    vec2      origin;
    Sprite    sprite;
    bool      hori_flip;
    bool      vert_flip;
};

#define SPRITE_COMP_INIT                                                                                       \
    {                                                                                                          \
        .color = COLOR_WHITE_INIT                                                                              \
    }

struct LocalTransformMatrixComp
{
    mat3 value;
};

struct WorldTransformMatrixComp
{
    mat3 value;
};

struct AnimationComp
{
    const Animation* ref;
    float            elapsed_time;
};

struct AnimationPoolComp
{
    const Animation* animations;
    u32              count;
};

struct DrawOrderComp
{
    float value;
};

struct VelocityComp
{
    vec2  value;
    float slowdown_factor;
};

struct MaterialComp
{
    const Material* ref;
    bool            shared;
};

struct ControllerComp
{
    vec2 desired_direction;
    bool in_action;
    u32  action;
};

struct InputComp
{
    int dummy;
};

struct SpeedComp
{
    float value;
};

struct CharacterAnimationControllerComp
{
    u32 state;
};

struct FacingDirectionComp
{
    vec2 value;
    bool frezzed;
};

struct HolderComp
{
    ecs_entity_t holder;
};

struct RelationshipComp
{
    ecs_entity_t parent;
    ecs_entity_t next;  /* next sibling */
    ecs_entity_t prev;  /* previous sibling */
    ecs_entity_t first; /* first child */
};

#define RELATIONSHIP_COMP_INIT                                                                                 \
    {                                                                                                          \
        ECS_NULL_ENT, ECS_NULL_ENT, ECS_NULL_ENT, ECS_NULL_ENT                                                 \
    }

struct DropComp
{
    int dummy;
};

struct HitBoxComp
{
    int  proxy_id;
    vec2 size;
    vec2 anchor;
    u32  mask;
    u32  category;
};

struct NameComp
{
    char value[64];
};

struct WorldTransformMatrixChangedTag
{
    int dummy;
};

struct EquipedWeapon
{
    u32 weapon_id;
};

struct EquipedSpell
{
    u32 spell_id;
};

struct EntityTag
{
    u32 value;
};

struct RotatorComp
{
    float speed;
};

enum InteractCommand
{
    ICMD_TALK,
    ICMD_BUY,
    ICMD_SELL,
    ICMD_CNT,
};

struct InteractableComp
{
    u32 commnads[ICMD_CNT];
    u32 command_count;
};

struct AffineAnimComp
{
    const struct AffineAnimCmd* cmds;
    float                       timer;
    float                       initial_x;
    float                       initial_y;
    float                       initial_scale_x;
    float                       initial_scale_y;
    float                       initial_rotation;
    bool                        should_reset_to_initial_state;
    bool                        realative_to_initial_state;
    int                         current_frame;
    bool                        finished;
    void (*finished_callback)(void*, ecs_Registry*, ecs_entity_t);
    void* finished_callback_ctx;
    bool  invert;
};

struct RefComp
{
    ecs_entity_t hand;
    ecs_entity_t weapon;
};

struct SwingWeaponComp
{
    int dummy;
};

struct CastSpellComp
{
    bool  start_casting;
    bool  cast_ended;
    float cooldown_timer;
};

struct DefenseComp
{
    s16 physical;
    s16 ice;
    s16 lighting;
    s16 fire;
    s16 dark;
    s16 holy;
};

struct HealthPoolComp
{
    s16 current;
    s16 max;
};

struct ManaPoolComp
{
    s16 current;
    s16 max;
};

struct PhysicalAtkPowerComp
{
    s16 value;
};

struct MagicalAtkPowComp
{
    s16 value;
};

struct EquipmentSlotsComp
{
    union
    {
        struct
        {
            u32 weapon;
            u32 armor;
            u32 boots;
            u32 ring1;
            u32 ring2;
        };
        u32 as_array[EQM_TYPE_CNT];
    };
};

struct BaseStatsComp
{
    s16 strength;
    s16 intelligent;
    s16 vitality;
    s16 luck;
};

struct StatModifiersComp
{
    struct StatModifier buff;
    struct StatModifier debuff;
    struct StatModifier equipments[EQM_TYPE_CNT];
};

#define STAT_MOFIFIERS_COMP_INIT                                                                               \
    {                                                                                                          \
        0                                                                                                      \
    }

struct StatModifiersChangedTag
{
    int dummy;
};

struct BaseStatsChangedTag
{
    int dummy;
};

struct EquipmentChangedTag
{
    u32 mask;
};

#endif // COMPONENTS_H
