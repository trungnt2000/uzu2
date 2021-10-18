#include "components.h"
#include "entity.h"
#include "entity_factory.h"

static Animation     s_animation_job_lizzard[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_wizzard[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_knight[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_hunter[CHARACTER_ANIMATION_CNT];
static const Sprite* s_sprite_wpn_anime_swd;

static const struct BaseStats s_base_stats_lizzard = {
    .strength    = 1,
    .intelligent = 1,
    .vitality    = 1,
    .luck        = 1,
};

static const struct BaseStats s_base_stats_wizzard = {
    .strength    = 1,
    .intelligent = 1,
    .vitality    = 1,
    .luck        = 1,
};

static const struct BaseStats s_base_stats_knight = {
    .strength    = 1,
    .intelligent = 1,
    .vitality    = 1,
    .luck        = 1,
};

static const struct BaseStats s_base_stats_hunter = {
    .strength    = 1,
    .intelligent = 1,
    .vitality    = 1,
    .luck        = 1,
};

struct CharacterParams
{
    float            x;
    float            y;
    const Animation* animations;
    struct BaseStats base_stats;
};

int
entity_factory_init(const Resources* resources)
{
    const SpriteSheet* spritesheet = &resources->spritesheet; // a small shortcut

    AnimationTemplate idle_tmpl = { 0 };
    AnimationTemplate walk_tmpl = { 0 };
    AnimationTemplate hurt_tmpl = { 0 };

    idle_tmpl.column_count   = 4;
    idle_tmpl.row_count      = 1;
    idle_tmpl.offset_x       = 16;
    idle_tmpl.offset_y       = 0;
    idle_tmpl.sprite_height  = 28;
    idle_tmpl.sprite_width   = 16;
    idle_tmpl.frame_duration = 0.1f;

    walk_tmpl.column_count   = 4;
    walk_tmpl.row_count      = 1;
    walk_tmpl.sprite_height  = 28;
    walk_tmpl.sprite_width   = 16;
    walk_tmpl.frame_duration = 0.1f;
    walk_tmpl.offset_x       = idle_tmpl.sprite_width * 5;
    walk_tmpl.offset_y       = 0;

    hurt_tmpl.column_count   = 1;
    hurt_tmpl.row_count      = 0;
    hurt_tmpl.frame_duration = 1.f;
    hurt_tmpl.offset_x       = 0;
    hurt_tmpl.offset_y       = 0;
    hurt_tmpl.sprite_height  = 28;
    hurt_tmpl.sprite_width   = 16;

    const Sprite* lizzard_sprite = sprite_sheet_get(spritesheet, "character_lizzard.png");
    if (lizzard_sprite == NULL)
        LEAVE_ERROR(-1, "lizzard.png not found\n");

    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_IDLE], lizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_WALK], lizzard_sprite, &walk_tmpl);
    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_HURT], lizzard_sprite, &hurt_tmpl);

    const Sprite* wizzard_sprite = sprite_sheet_get(spritesheet, "character_wizzard.png");
    if (wizzard_sprite == NULL)
        LEAVE_ERROR(-1, "wizard.png not found\n");

    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_IDLE], wizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_WALK], wizzard_sprite, &walk_tmpl);
    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_HURT], wizzard_sprite, &hurt_tmpl);

    const Sprite* knight_sprite = sprite_sheet_get(spritesheet, "character_knight.png");
    if (knight_sprite == NULL)
        LEAVE_ERROR(-1, "knight.png not found\n");

    animation_init_w_sprite(&s_animation_job_knight[CHARACTER_ANIMATION_IDLE], knight_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_knight[CHARACTER_ANIMATION_WALK], knight_sprite, &walk_tmpl);
    animation_init_w_sprite(&s_animation_job_knight[CHARACTER_ANIMATION_HURT], knight_sprite, &hurt_tmpl);

    s_sprite_wpn_anime_swd = sprite_sheet_get(spritesheet, "anime_sword.png");
    if (s_sprite_wpn_anime_swd == NULL)
        LEAVE_ERROR(-1, "anime_sword.png is not found in provided sprite sheet!\n");

    return 0;
}

void
entity_factory_shutdown(void)
{
    for (int i = 0; i < CHARACTER_ANIMATION_CNT; ++i)
        animation_destroy(&s_animation_job_lizzard[i]);
}

static ecs_entity_t
create_hand(ecs_Registry* registry, float anchor_x, float anchor_y)
{
    ecs_entity_t hand = ecs_create(registry);

    ecs_addv(registry, hand, TransformComp, { .position = { anchor_x, anchor_y }, .scale = { 1.f, 1.f } });
    ecs_addv(registry, hand, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, hand, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, hand, TransformChangedTag, { 0 });
    ecs_addv(registry, hand, NameComp, { "hand" });

    return hand;
}

struct BasicSpriteParams
{
    float               x;
    float               y;
    const Animation*    animations;
    enum EntityTagValue tag;
};

static ecs_entity_t
create_basic_sprite(ecs_Registry* registry, const struct BasicSpriteParams* params)
{
    ecs_entity_t basic_sprite = ecs_create(registry);
    ecs_addv(registry,
             basic_sprite,
             TransformComp,
             { .scale = { 1.f, 1.f }, .position = { params->x, params->y } });
    ecs_addv(registry, basic_sprite, SpriteComp, { .color = COLOR_WHITE_INIT, .origin = { 8.f, 28.f } });
    ecs_addv(registry, basic_sprite, AnimationComp, { .ref = &params->animations[CHARACTER_ANIMATION_IDLE] });
    ecs_addv(registry,
             basic_sprite,
             AnimationPoolComp,
             { .animations = params->animations, .count = CHARACTER_ANIMATION_CNT });
    ecs_addv(registry, basic_sprite, CharacterAnimationControllerComp, { CHARACTER_ANIMATION_IDLE });
    ecs_addv(registry, basic_sprite, VelocityComp, { GLM_VEC2_ZERO_INIT, 0.25f });
    ecs_addv(registry, basic_sprite, FacingDirectionComp, { { 1.f, 0.f }, false });
    ecs_addv(registry, basic_sprite, ControllerComp, { 0 });
    ecs_addv(registry, basic_sprite, SpeedComp, { 10.f * 16.f });
    ecs_addv(registry, basic_sprite, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, basic_sprite, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_add(registry, basic_sprite, TransformChangedTag);

    return basic_sprite;
}

static void
copy_character_params_to_sprite_params(const struct CharacterParams* character_params,
                                       struct BasicSpriteParams*     basic_sprite_params)
{
    basic_sprite_params->x          = character_params->x;
    basic_sprite_params->y          = character_params->y;
    basic_sprite_params->animations = character_params->animations;
}

static void
init_base_stats_component(struct BaseStatsComp* component, const struct BaseStats* base_stats)
{
    component->intelligent = base_stats->intelligent;
    component->luck        = base_stats->luck;
    component->vitality    = base_stats->vitality;
    component->strength    = base_stats->strength;
}

static ecs_entity_t
create_character(ecs_Registry* registry, const struct CharacterParams* params)
{
    struct BasicSpriteParams sprite_params = { 0 };
    copy_character_params_to_sprite_params(params, &sprite_params);
    ecs_entity_t character = create_basic_sprite(registry, &sprite_params);

    ecs_add(registry, character, InputComp);
    ecs_addv(registry,
             character,
             HitBoxComp,
             { .size = { 16.f, 28.f }, .anchor = { 8.f, 28.f }, .mask = ENEMY_MASK, .category = PLAYER_MASK });

    ecs_entity_t hand = ett_add_child(registry, character, create_hand(registry, 15, -6));

    ecs_addv(registry, character, RefComp, { .weapon = ECS_NULL_ENT, .hand = hand });

    ecs_add(registry, character, SwingWeaponComp);

    init_base_stats_component(ecs_add(registry, character, BaseStatsComp), &params->base_stats);

    return character;
}

ecs_entity_t
create_anime_sword(ecs_Registry* registry)
{
    ecs_entity_t anime_sword = ecs_create(registry);

    const float sw = (float)s_sprite_wpn_anime_swd->rect.w;
    const float sh = (float)s_sprite_wpn_anime_swd->rect.h;

    ecs_addv(registry,
             anime_sword,
             TransformComp,
             { .scale = { 1.f, 1.f }, .position = { 0.f, 0.f }, .rotation = 90.f });
    ecs_addv(registry, anime_sword, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, anime_sword, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry,
             anime_sword,
             SpriteComp,
             {
                 .sprite = *s_sprite_wpn_anime_swd,
                 .color  = COLOR_WHITE_INIT,
                 .origin = { sw / 2.f, sh },
             });

    ecs_add(registry, anime_sword, TransformChangedTag);

    return anime_sword;
}

ecs_entity_t
spawn_player(ecs_Registry* registry, float x, float y, const struct PlayerData* player_data)
{
    struct CharacterParams params = { 0 };
    params.x                      = x;
    params.y                      = y;

    SDL_memcpy(&params.base_stats, &player_data->stats, sizeof(struct BaseStats));
    switch (player_data->job)
    {
    case JOB_LIZZARD:
        params.animations = s_animation_job_lizzard;
        break;
    case JOB_KNIGHT:
        params.animations = s_animation_job_knight;
        break;
    case JOB_WIZZARD:
        params.animations = s_animation_job_wizzard;
        break;
    case JOB_HUNTER:
        params.animations = s_animation_job_hunter;
    default:
        ASSERT(0);
    }

    ecs_entity_t player = create_character(registry, &params);
    ecs_addv(registry, player, EntityTag, { ENTITY_TAG_PLAYER });
    return player;
}

ecs_entity_t
spawn_npc(ecs_Registry* registry, const struct NpcParams* params)
{
    ecs_entity_t npc = ecs_create(registry);

    const float sw = (float)params->animations[0].frames[0].rect.w;
    const float sh = (float)params->animations[0].frames[0].rect.h;

    ecs_addv(registry, npc, TransformComp, { .scale = { 1.f, 1.f }, .position = { params->x, params->y } });
    ecs_addv(registry, npc, SpriteComp, { .color = COLOR_WHITE_INIT, .origin = { sw / 2.f, sh } });
    ecs_addv(registry, npc, AnimationComp, { .ref = &params->animations[NPC_ANIMATION_IDLE] });
    ecs_addv(registry,
             npc,
             AnimationPoolComp,
             { .animations = params->animations, .count = NPC_ANIMATION_CNT });
    ecs_addv(registry, npc, CharacterAnimationControllerComp, { CHARACTER_ANIMATION_IDLE });
    ecs_addv(registry, npc, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry,
             npc,
             HitBoxComp,
             { .size = { sw, sh }, .anchor = { sw / 2.f, sh / 2.f }, .category = INTERACTABLE_MASK });
    ecs_addv(registry, npc, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_add(registry, npc, TransformChangedTag);

    SDL_memcpy(ecs_add(registry, npc, InteractableComp)->commnads, params->commands, sizeof(params->commands));
    return npc;
}

ecs_entity_t
spawn_weapon(ecs_Registry* registry, const Sprite* sprite)
{
    ecs_entity_t weapon = ecs_create(registry);

    const float sw = (float)sprite->rect.w;
    const float sh = (float)sprite->rect.h;

    ecs_addv(registry,
             weapon,
             TransformComp,
             { .scale = { 1.f, 1.f }, .position = { 0.f, 0.f }, .rotation = 90.f });
    ecs_addv(registry, weapon, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, weapon, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry,
             weapon,
             SpriteComp,
             {
                 .sprite = *sprite,
                 .color  = COLOR_WHITE_INIT,
                 .origin = { sw / 2.f, sh },
             });

    ecs_add(registry, weapon, TransformChangedTag);

    return weapon;
}

ecs_entity_t
spawn_testing_npc(ecs_Registry* registry, float x, float y)
{
}
