// ngotrung Sat 24 Jul 2021 11:58:27 PM +07
#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "cglm/cglm.h"
#include "ecs.h"
#include "graphics.h"
#include "toolbox/common.h"

enum
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
    HandComp,
    LocalTransformMatrixComp, // caculated when transform component changed
    WorldTransformMatrixComp, //
    RelationshipComp,
    HitBoxComp,
    DestroyTag,
    /* Stats components */
    SpeedComp,
    COMPONENT_CNT
};

extern const struct ecs_TypeTraits g_comp_traits[];

struct TransformComp
{
    vec3  position;
    vec2  scale;
    float rotation;
};

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
    int              count;
};

struct DrawOrderComp
{
    float value;
};

struct VelocityComp
{
    vec2 value;
};

struct MaterialComp
{
    const Material* ref;
    bool            shared;
};

struct ControllerComp
{
    vec2 desired_direction;
};

struct InputComp
{
    int dummy;
};

struct SpeedComp
{
    float value;
};

typedef enum CharacterAnimationState
{
    CHARACTER_ANIMATION_IDLE,
    CHARACTER_ANIMATION_WALK,
    CHARACTER_ANIMATION_HURT,
    CHARACTER_ANIMATION_CNT,
} CharacterAnimationState;

struct CharacterAnimationControllerComp
{
    CharacterAnimationState state;
};

struct FacingDirectionComp
{
    vec2 value;
    bool frezzed;
};

struct HandComp
{
    ecs_entity_t weapon;
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

#define RELATIONSHIP_COMP_INIT_EMPTY                                                                           \
    {                                                                                                          \
        .parent = ECS_NULL_ENT, .next = ECS_NULL_ENT, .prev = ECS_NULL_ENT, .first = ECS_NULL_ENT              \
    }

#endif // COMPONENTS_H
