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
#include "ui/inventory.h"

static Shader        s_shader;
static OthoCamera    s_camera;
static ecs_Registry* s_registry;
static Resources     s_resources;
static Texture       s_texture;
static Sprite        s_sprite;

static void
process_input(void* UNUSED arg, UNUSED u32 current_state, UNUSED u32 previous_state)
{
    if (current_state & BTN_MSK_INVENTORY)
        ui_inventory_show();
}

static ecs_entity_t
create_sprite(ecs_Registry* registry, const Sprite* sprite, float x, float y)
{
    ecs_entity_t ett = ecs_create(registry);

    ecs_addv(registry, ett, TransformComp, { .position = { x, y }, .scale = { 1.f, 1.f } });
    ecs_addv(registry, ett, TransformChangedTag, { 0 });
    ecs_addv(registry, ett, SpriteComp, { .sprite = *sprite, .color = COLOR_WHITE_INIT });
    ecs_addv(registry, ett, MaterialComp, { .ref = NULL });

    return ett;
}

static void
initialize_test(ecs_Registry* registry)
{
    ecs_entity_t lizzard = create_lizzard(registry);
    ecs_entity_t sword0  = create_anime_sword(registry);
    ecs_entity_t sword1  = create_anime_sword(registry);

    ecs_get(registry, sword1, SpriteComp)->hori_flip = true;

    ett_tx_set_position(registry, lizzard, (vec2){ 50.f, 50.f });

    ett_tx_set_rotation(registry, sword0, 35.f);
    ett_tx_set_rotation(registry, sword1, -45);

    ett_rs_add_child(registry, lizzard, sword0);
    ett_rs_add_child(registry, lizzard, sword1);

    inventory_add_item(ITEM_BIG_BLUE_FLASK, 5);
    inventory_add_item(ITEM_BIG_RED_FLASK, 5);
}

static void
initialize_systems()
{
    ems_init();
    system_rendering_sprite_init(s_registry);
    system_box_collision_init(s_registry);
}

int
scene_main_create(void)
{
    otho_camera_reset(&s_camera, 160, 120, 320, 240);

    if (shader_load(&s_shader, "res/shader/sprite.vert", "res/shader/sprite.frag") != 0)
        LEAVE_ERROR(-1, "Load shader fail \n");

    if (load_resources(&s_resources) != 0)
        LEAVE_ERROR(-1, "Load resources fail\n");

    if (EF_init(&s_resources) != 0)
        LEAVE_ERROR(-1, "Failed to init entity factory module!\n");

    if (item_init(&s_resources.spritesheet) != 0)
        LEAVE_ERROR(-1, "Failed to init item module\n");

    if (ui_inventory_init(&s_resources) != 0)
        LEAVE_ERROR(-1, "Init inventory module failed\n");

    s_registry = ecs_registry_create(g_comp_traits, COMPONENT_CNT);

    initialize_systems();
    initialize_test(s_registry);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 0;
}

void
scene_main_destroy(void)
{
    shader_destroy(&s_shader);
    EF_shutdown();
    system_rendering_sprite_shutdown();
    ui_inventory_shutdown();
    release_resources(&s_resources);
    ecs_registry_free(s_registry);
}

static void
update_game_world(float delta_time)
{
    system_input_update(s_registry, &s_camera);
    system_controller_system(s_registry, delta_time);
    system_motion_update(s_registry, delta_time);
}

void
draw_delta_time(float delta_time)
{
    mat4 proj_matrix;
    char buf[255];
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    sprintf(buf, "%.2f", delta_time);
    begin_text(NULL, proj_matrix);
    {
        draw_text(buf, 0, 0, COLOR_BLUE);
    }
    end_text();
}

static void
draw_scene(float delta_time)
{
    mat4 view_proj_matrix;
    mat4 proj_matrix;
    otho_camera_get_view_projection_matrix(&s_camera, view_proj_matrix);
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    system_rendering_character_animation_update(s_registry);
    system_rendering_animation_update(s_registry, delta_time);
    system_rendering_transform_update(s_registry);

    begin_sprite(view_proj_matrix);
    {
        system_rendering_sprite_update(s_registry);
    }
    end_sprite();

    draw_delta_time(delta_time);

    system_tag_update(s_registry);
}

static void
draw_ui(float delta_time)
{
    mat4 proj_matrix;
    otho_camera_get_projection_matrix(&s_camera, proj_matrix);

    shader_bind(&s_shader);
    shader_upload_view_project_matrix(&s_shader, proj_matrix);

    begin_text(NULL, proj_matrix);
    begin_sprite(proj_matrix);
    {
        ui_inventory_draw(delta_time);
    }
    end_sprite();
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
        draw_text(buf, 0, 0, COLOR(0x00, 0xff, 0x00, 0xff));
    }
    end_text();
}

void
scene_main_tick(float delta_time)
{
    glDisable(GL_DEPTH_TEST);
    update_game_world(delta_time);
    draw_scene(delta_time);
    draw_ui(delta_time);
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
