#include "scn_main.h"
#include "components.h"
#include "config.h"
#include "constances.h"
#include "ecs.h"
#include "engine.h"
#include "entity.h"
#include "entity_factory.h"
#include "equipment.h"
#include "graphics.h"
#include "graphics/gl.h"
#include "input.h"
#include "inventory.h"
#include "item.h"
#include "level_loader.h"
#include "map.h"
#include "map_render.h"
#include "message.h"
#include "system_logic.h"
#include "system_render.h"
#include "ui/equipment.h"
#include "ui/inventory.h"
#include "ui/menu.h"

static Shader        s_shader;
static OthoCamera    s_camera;
static ecs_Registry* s_registry;
static Resources     s_resources;
static Sprite        s_sprite;

static char s_pending_level[64];
static bool s_has_pending_level;
static bool s_paused;

struct MainContext g_main_ctx;
struct SaveBlock   g_save_block;

static void
process_input(UNUSED void* ctx)
{
    if (button_just_pressed(BTN_INVENTORY))
        ui_inventory_show();
    system_input_update(s_registry, &s_camera);
}

static bool
on_player_hit_enemy(SDL_UNUSED void* ctx, const void* data)
{
    const struct MsgPlayerHitEnemy* msg = data;

    struct SpriteComp* sprite_comp = ecs_get(s_registry, msg->enemy, SpriteComp);

    glm_vec4_copy(COLOR_RED, sprite_comp->color);

    return false;
}

static void
init_inventory_test(void)
{
    inv_add_item(ITEM_BIG_BLUE_FLASK, 200);
    inv_add_item(ITEM_BIG_RED_FLASK, 5);
    inv_add_item(ITEM_BIG_RED_FLASK, 5);
    inv_add_item(ITEM_BIG_BLUE_FLASK, 10);
    inv_add_item(ITEM_IRON_BOOTS, 1);
    inv_add_item(ITEM_LEATHER_ARMOR, 1);
    inv_add_item(ITEM_SAPPHIRE_RING, 1);
    inv_add_item(ITEM_WOODEN_RING, 1);
    inv_add_item(ITEM_FIRE_BALL, 1);
    inv_add_item(ITEM_RUBY_RING, 4);
}

static void
initialize_test(ecs_Registry* registry)
{
    struct PlayerData* player_data = &g_save_block.player_data;

    player_data->job   = JOB_KNIGHT;
    player_data->stats = (struct BaseStats){
        .strength    = 1,
        .vitality    = 1,
        .intelligent = 1,
        .luck        = 1,
    };

    ecs_entity_t lizzard = spawn_player(s_registry, 16.f, 32.f, &g_save_block.player_data);
    ecs_entity_t sword0  = create_anime_sword(registry);

    g_main_ctx.player = lizzard;

    ett_attach_weapon(registry, lizzard, sword0);

    ecs_addv(registry, lizzard, NameComp, { "lizzard" });
    ecs_addv(registry, sword0, NameComp, { "sword0" });

    eqm_equip_armor(registry, lizzard, EQM_LEATHER_ARMOR);
    eqm_equip_ring1(registry, lizzard, EQM_SAPPHIRE_RING);
    eqm_equip_boots(registry, lizzard, EQM_IRON_BOOTS);
    eqm_equip_ring2(registry, lizzard, EQM_WOODEN_RING);

    init_inventory_test();

    ems_connect(MsgPlayerHitEnemy, on_player_hit_enemy, NULL, 0);
}

static void
make_all_enemies_white(ecs_Registry* registry)
{
    struct SpriteComp* sprite_comp_array;
    ecs_size_t         count;

    ecs_raw(registry, SpriteComp, NULL, (void**)&sprite_comp_array, &count);
    for (ecs_size_t i = 0; i < count; ++i)
    {
        glm_vec4_copy(COLOR_WHITE, sprite_comp_array[i].color);
    }
}

static void
initialize_systems(void)
{
    ems_init();
    map_renderer_init();
    system_rendering_sprite_init(s_registry);
    system_box_collision_init(s_registry);
    system_collision_manager_init(s_registry);
}

static void
do_load_map(void)
{
    SDL_strlcpy(g_save_block.current_map, s_pending_level, (sizeof g_save_block.current_map) - 1);
    load_level(s_pending_level);
    s_has_pending_level = false;
}

void
request_load_level(const char* level_name)
{
    s_has_pending_level = true;
    SDL_strlcpy(s_pending_level, level_name, (sizeof s_pending_level) - 1);
}

int
scene_main_create(void)
{
    otho_camera_reset(&s_camera, 160, 120, 320, 240);

    if (shader_load(&s_shader, "res/shader/sprite.vert", "res/shader/sprite.frag") != 0)
        LEAVE_ERROR(-1, "Load shader fail \n");

    if (load_resources(&s_resources) != 0)
        LEAVE_ERROR(-1, "Load resources fail\n");

    if (entity_factory_init(&s_resources) != 0)
        LEAVE_ERROR(-1, "Failed to init entity factory module!\n");

    if (item_init(&s_resources.spritesheet) != 0)
        LEAVE_ERROR(-1, "Failed to init item module\n");

    s_registry = ecs_registry_create(g_comp_traits, COMPONENT_CNT);

    if (ui_inventory_init(&s_resources, s_registry) != 0)
        LEAVE_ERROR(-1, "Init inventory module failed\n");

    ui_menu_init(&s_resources.font_8px);
    ui_equipment_init(&s_resources.font_8px);

    initialize_systems();
    initialize_test(s_registry);

    load_level("demon_ruin");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 0;
}

void
scene_main_destroy(void)
{
    map_renderer_fini();
    shader_destroy(&s_shader);
    entity_factory_shutdown();
    system_rendering_sprite_shutdown();
    ui_inventory_shutdown();
    release_resources(&s_resources);
    ecs_registry_free(s_registry);
}

static void
update_game_world(float delta_time)
{
    system_controller_system(s_registry, delta_time);
    system_motion_update(s_registry, delta_time);
    system_swing_weapon_update(s_registry);
    system_affine_animation_update(s_registry, delta_time);
    system_rendering_transform_update(s_registry);
    system_box_collision_update(s_registry);
    system_stats_update(s_registry);
    system_interactable_entity_update(s_registry);
}

static void
draw_delta_time(float delta_time, float used)
{
    mat4 proj_matrix;
    char buf[255];
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    sprintf(buf, "delta_time: %.4f update_time: %.4f", delta_time, used);
    begin_text(NULL, proj_matrix);
    {
        draw_text_ex(buf, NULL, 0, 0, 0.25f, TEXT_ALIGN_LEFT, COLOR(0x00, 0xff, 0x00, 0xff));
    }
    end_text();
}

static UNUSED void
draw_statistics(const RenderStatistics* statistics)
{
    mat4 proj_matrix;
    char buf[255];
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    sprintf(buf, "draw calls: %u\nvertices: %u", statistics->draw_call_count, statistics->vertex_count);
    begin_text(&s_shader, proj_matrix);
    {
        // draw_text(buf, 0, 32, COLOR(0x00, 0xff, 0x00, 0xff));
        draw_text_ex(buf, NULL, 0, 8, 0.25f, TEXT_ALIGN_LEFT, COLOR(0x00, 0xff, 0x00, 0xff));
    }
    end_text();
}

static void
draw_scene(float delta_time)
{
    mat4 view_proj_matrix;
    mat4 proj_matrix;

    system_camera_update(s_registry, g_main_ctx.player, &s_camera);

    otho_camera_get_view_projection_matrix(&s_camera, view_proj_matrix);
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    system_rendering_character_animation_update(s_registry);
    system_rendering_animation_update(s_registry, delta_time);

    map_render(&s_camera);

    begin_sprite(view_proj_matrix);
    {
        system_rendering_sprite_update(s_registry);
    }
    end_sprite();
    struct RenderStatistics statistics;
    sprite_renderer_get_statistis(&statistics);
    draw_statistics(&statistics);
}

static void
draw_ui(float delta_time)
{
    mat4 proj_matrix;
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    begin_text(NULL, proj_matrix);
    begin_sprite(proj_matrix);
    {
        ui_inventory_draw(delta_time);
    }
    end_sprite();
    end_text();

    ui_menu_draw(proj_matrix);
    ui_equipment_draw(s_registry, g_main_ctx.player, proj_matrix);
}

void
scene_main_tick(float delta_time)
{
    u32 start = SDL_GetTicks();
    glDisable(GL_DEPTH_TEST);
    if (!s_paused)
    {
        update_game_world(delta_time);
        map_tick();
    }
    draw_scene(delta_time);
    draw_ui(delta_time);
    system_tag_update(s_registry);
    make_all_enemies_white(s_registry);

    u32 used = SDL_GetTicks() - start;
    draw_delta_time(delta_time, (float)used / 1000.f);
}

void
scene_main_enter(void)
{
    input_push(INPUT_CALLBACK1(process_input));
}

void
scene_main_leave(void)
{
    input_pop();
}

void
scene_main_receive_event(UNUSED const SDL_Event* event)
{
}

void
scene_main_pause()
{
    s_paused = true;
}

void
scene_main_resume()
{
    s_paused = false;
}
