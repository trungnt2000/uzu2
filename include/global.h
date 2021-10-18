// ngotrung Sun 10 Oct 2021 12:43:20 AM +07
#ifndef GLOBAL_H
#define GLOBAL_H

#include "cglm/cglm.h"
#include "constances.h"
#include "ecs.h"

enum Action
{
    ACTION_NONE,
    ACTION_ATTACK
};

enum EquipmentType
{
    EQM_TYPE_WEAPON,
    EQM_TYPE_ARMOR,
    EQM_TYPE_BOOTS,
    EQM_TYPE_RING,
    EQM_TYPE_CNT,
};

enum EquipmentSlot
{
    EQM_SLOT_WEAPON,
    EQM_SLOT_ARMOR,
    EQM_SLOT_BOOTS,
    EQM_SLOT_RING1,
    EQM_SLOT_RING2,
    EQM_SLOT_CNT,
};

enum EquimentId
{
    EQM_ANIME_SWORD,
    EQM_RED_STAFF,
    EQM_CLEAVER,
    EQM_WOODEN_RING,
    EQM_SAPPHIRE_RING,
    EQM_IRON_BOOTS,
    EQM_LEATHER_ARMOR,
    EQM_RUBY_RING,
    EQM_CNT,
};

#define EQM_NULL EQM_CNT

enum AttackAction
{
    ATTACK_ACTION_NONE,
    ATTACK_ACTION_SWING,
    ATTACK_ACTION_CAST,
    ATTACK_ACTION_THUST,
};

enum CharacterAnimationStateValue
{
    CHARACTER_ANIMATION_IDLE,
    CHARACTER_ANIMATION_WALK,
    CHARACTER_ANIMATION_HURT,
    CHARACTER_ANIMATION_CNT
};

enum EnemyAnimationStateValue
{
    ENEMY_ANIMATION_IDLE,
    ENEMY_ANIMATION_WALK,
    ENEMY_ANIMATION_CNT
};

enum EntityTagValue
{
    ENTITY_TAG_ENEMY,
    ENTITY_TAG_PLAYER,
    ENTITY_TAG_PICKUPABLE,
    ENTITY_TAG_INTERACTABLE,
    ENTITY_TAG_WEAPON,
    ENTITY_TAG_PROJECTILE,
    ENTITY_TAG_CNT
};

enum NpcAnimationState
{
    NPC_ANIMATION_IDLE,
    NPC_ANIMATION_WALK,
    NPC_ANIMATION_CNT,
};

enum InteractCommand
{
    INTERACT_CMD_TALK,
    INTERACT_CMD_BUY,
    INTERACT_CMD_SELL,
    INTERACT_CMD_CNT,
};

#define INTERACT_CMD_NONE INTERACT_CMD_CNT

#define ENTITY_TAG_UNKNOWN ENTITY_TAG_CNT
#define ENEMY_MASK BIT(ENTITY_TAG_ENEMY)
#define PLAYER_MASK BIT(ENTITY_TAG_PLAYER)
#define INTERACTABLE_MASK BIT(ENTITY_TAG_INTERACTABLE)

enum JobId
{
    JOB_LIZZARD,
    JOB_WIZZARD,
    JOB_HUNTER,
    JOB_KNIGHT,
    JOB_CNT
};

#define JOB_LIZZARD_MASK (1 << JOB_LIZZARD)
#define JOB_WIZZARD_MASK (1 << JOB_WIZZARD)
#define JOB_HUNTER_MASK (1 << JOB_HUNTER)
#define JOB_KNIGHT_MASK (1 << JOB_KNIGHT)

enum ItemId
{
    /* consumable */
    ITEM_RED_FLASK,
    ITEM_BLUE_FLASK,
    ITEM_BIG_RED_FLASK,
    ITEM_BIG_BLUE_FLASK,

    /* spells */
    ITEM_FIRE_BALL,
    ITEM_ICE_SPIKE,

    /* equipments */
    ITEM_ANIME_SWORD,
    ITEM_WOODEN_RING,
    ITEM_SAPPHIRE_RING,
    ITEM_IRON_BOOTS,
    ITEM_LEATHER_ARMOR,
    ITEM_RUBY_RING,

    ITEM_CNT
};

struct AffineAnimCmd
{
    int   type;
    vec3  position;
    vec2  scale;
    float rotation;
    float duration;
};

#define AFFINE_ANIM_CMD(x, y, z, sx, sy, rot, d)                                                               \
    {                                                                                                          \
        .type = 0, .position = { x, y, z }, .scale = { sx, sy }, .rotation = rot, .duration = d                \
    }

#define AFFINE_ANIM_CMD_END                                                                                    \
    {                                                                                                          \
        .type = -1                                                                                             \
    }

enum DamageType
{
    DAMAGE_TYPE_PHYSICAL,
    DAMAGE_TYPE_ICE,
    DAMAGE_TYPE_FIRE,
    DAMAGE_TYPE_LIGHTING,
    DAMAGE_TYPE_DARK,
    DAMAGE_TYPE_HOLY,
    DAMAGE_TYPE_CNT,
};

/* this stats can increase when leve up */
struct BaseStats
{
    s16 strength;
    s16 intelligent;
    s16 vitality;
    s16 luck;
};

struct Defense
{
    s16 physical;
    s16 ice;
    s16 lighting;
    s16 fire;
    s16 holy;
    s16 dark;
};

#define MODIFIER_TYPE_SHIFT (12)
#define MODIFIER_VALUE_SHIFT (0)
#define MODIFIER_TYPE_MASK (0xf)
#define MODIFIER_VALUE_MASK (0xfff)

enum ModifierType
{
    MODIFIER_TYPE_FLAT,
    MODIFIER_TYPE_PERCENT,
};

#define MODIFIER_INIT(type, value) (((type) << MODIFIER_TYPE_SHIFT) | ((value) << MODIFIER_VALUE_SHIFT))
#define MODIFIER_FLAT_INIT(value) MODIFIER_INIT(MODIFIER_TYPE_FLAT, value)
#define MODEFIER_PERCENT_INIT(value) MODIFIER_INIT(MODIFIER_TYPE_PERCENT, value)
#define MODIFIER_TYPE(modifier) (((modifier) >> MODIFIER_TYPE_SHIFT) & MODIFIER_TYPE_MASK)
#define MODIFIER_VALUE(modifier) (((modifier) >> MODIFIER_VALUE_SHIFT) & MODIFIER_VALUE_MASK)

struct StatModifier
{
    bool active;

    s16              health;
    s16              mana;
    struct BaseStats base;
    struct Defense   defense;
};

struct Spell
{
    u32 item_id;
    void (*cast_func)(ecs_Registry* registry, ecs_entity_t entity);
    s16 cost;
    u32 vfx;
    u32 sfx;
};

struct PlayerData
{
    char name[64];
    u32  job;

    struct
    {
        u32 spell;
        u32 weapon;
        u32 armor;
        u32 ring1;
        u32 ring2;
        u32 boots;
    } equipment;

    s16 current_hit_points;
    s16 current_mana_points;

    float position_x;
    float position_y;

    struct BaseStats stats;
};

struct SaveBlock
{
    char              current_map[64];
    struct PlayerData player_data;
};

#endif // GLOBAL_H
