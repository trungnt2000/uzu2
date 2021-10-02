#include "components.h"
#include "entity_factory.h"

static Animation     s_animation_job_lizzard[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_wizzard[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_knight[CHARACTER_ANIMATION_CNT];
static Animation     s_animation_job_hunter[CHARACTER_ANIMATION_CNT];
static const Sprite* s_sprite_wpn_anime_swd;

typedef struct
{
    const Animation* animations;
} CharacterParams;

const static CharacterParams s_params_job_lizzard = { .animations = s_animation_job_lizzard };
const static CharacterParams s_params_job_wizzard = { .animations = s_animation_job_wizzard };
const static CharacterParams s_params_job_knight  = { .animations = s_animation_job_knight };

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

    const Sprite* lizzard_sprite = sprite_sheet_get(spritesheet, "lizzard.png");
    if (lizzard_sprite == NULL)
        LEAVE_ERROR(-1, "lizzard.png not found\n");

    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_IDLE], lizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_WALK], lizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_lizzard[CHARACTER_ANIMATION_HURT], lizzard_sprite, &hurt_tmpl);

    const Sprite* wizzard_sprite = sprite_sheet_get(spritesheet, "wizzard.png");
    if (wizzard_sprite == NULL)
        LEAVE_ERROR(-1, "wizard.png not found\n");

    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_IDLE], wizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_WALK], wizzard_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_wizzard[CHARACTER_ANIMATION_HURT], wizzard_sprite, &hurt_tmpl);

    const Sprite* knight_sprite = sprite_sheet_get(spritesheet, "knight.png");
    if (knight_sprite == NULL)
        LEAVE_ERROR(-1, "knight.png not found\n");

    animation_init_w_sprite(&s_animation_job_knight[CHARACTER_ANIMATION_IDLE], knight_sprite, &idle_tmpl);
    animation_init_w_sprite(&s_animation_job_knight[CHARACTER_ANIMATION_WALK], knight_sprite, &idle_tmpl);
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
make_character(ecs_Registry* registry, const CharacterParams* params)
{
    ecs_entity_t character = ecs_create(registry);
    ecs_addv(registry, character, TransformComp, { .scale = { 1.f, 1.f } });
    ecs_addv(registry, character, SpriteComp, { .color = COLOR_WHITE_INIT, .origin = { 8.f, 28.f } });
    ecs_addv(registry, character, MaterialComp, { .ref = NULL });
    ecs_addv(registry, character, AnimationComp, { .ref = &params->animations[CHARACTER_ANIMATION_IDLE] });
    ecs_addv(registry,
             character,
             AnimationPoolComp,
             { .animations = params->animations, .count = CHARACTER_ANIMATION_CNT });
    ecs_addv(registry, character, CharacterAnimationControllerComp, { CHARACTER_ANIMATION_IDLE });
    ecs_addv(registry, character, VelocityComp, { GLM_VEC2_ZERO_INIT });
    ecs_addv(registry, character, FacingDirectionComp, { { 1.f, 0.f }, false });
    ecs_addv(registry, character, ControllerComp, { 0 });
    ecs_addv(registry, character, SpeedComp, { 10.f * 16.f });
    ecs_addv(registry, character, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_addv(registry, character, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_add(registry, character, TransformChangedTag);
    ecs_add(registry, character, InputComp);
    ecs_addv(registry,
             character,
             HitBoxComp,
             { .size = { 16.f, 28.f }, .anchor = { 8.f, 28.f }, .mask = ENEMY_MASK, .category = PLAYER_MASK });
    return character;
}

ecs_entity_t
create_anime_sword(ecs_Registry* registry)
{
    ecs_entity_t anime_sword = ecs_create(registry);

    const float sw = (float)s_sprite_wpn_anime_swd->rect.w;
    const float sh = (float)s_sprite_wpn_anime_swd->rect.h;

    ecs_add_ex(registry,
               anime_sword,
               TransformComp,
               { .scale = { 1.f, 1.f }, .position = { 0.f, 0.f }, .rotation = 45.f });
    ecs_add_ex(registry, anime_sword, LocalTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_add_ex(registry, anime_sword, WorldTransformMatrixComp, { GLM_MAT3_IDENTITY_INIT });
    ecs_add_ex(registry,
               anime_sword,
               SpriteComp,
               {
                   .sprite = *s_sprite_wpn_anime_swd,
                   .color  = COLOR_WHITE_INIT,
                   .origin = { sw / 2.f, sh },
               });
    ecs_add_ex(registry, anime_sword, MaterialComp, { .ref = NULL });

    ecs_add(registry, anime_sword, TransformChangedTag);

    return anime_sword;
}

ecs_entity_t
create_lizzard(ecs_Registry* registry)
{
    return make_character(registry, &s_params_job_lizzard);
}
