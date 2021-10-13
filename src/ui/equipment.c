
#include "components.h"
#include "equipment_system.h"
#include "input.h"
#include "item.h"
#include "scn_main.h"

static bool s_visible;

static vec2 s_pos_weapon = { 32.f, 16.f };
static vec2 s_pos_armor  = { 32.f, 32.f };
static vec2 s_pos_boots  = { 32.f, 48.f };
static vec2 s_pos_ring1  = { 32.f, 64.f };
static vec2 s_pos_ring2  = { 32.f, 80.f };

static const Font* s_font_8px = NULL;

static void
close(void)
{
    s_visible = false;
    input_pop();
}

static void
process_input(UNUSED void* ctx)
{
    if (button_just_pressed(BTN_CANCEL))
    {
        close();
    }
}

void
ui_equipment_init(const Font* font_8px)
{
    s_font_8px = font_8px;
}

void
ui_equipment_show(void)
{
    s_visible = true;
    input_push(INPUT_CALLBACK1(process_input));
}

void
ui_equipment_draw(ecs_Registry* registry, ecs_entity_t player, mat4 projection_matrix)
{
    if (!s_visible)
        return;

    const struct EquipmentSlotsComp* slots = ecs_get(registry, player, EquipmentSlotsComp);

    ASSERT_MSG(slots, "Player does not have equipment component");

    begin_sprite(projection_matrix);
    {
        if (slots->weapon != EQM_NULL)
            draw_sprite(EQUIPMENT_ICON(slots->weapon), NULL, s_pos_weapon, COLOR_WHITE);

        if (slots->armor != EQM_NULL)
            draw_sprite(EQUIPMENT_ICON(slots->armor), NULL, s_pos_armor, COLOR_WHITE);

        if (slots->boots != EQM_NULL)
            draw_sprite(EQUIPMENT_ICON(slots->boots), NULL, s_pos_boots, COLOR_WHITE);

        if (slots->ring1 != EQM_NULL)
            draw_sprite(EQUIPMENT_ICON(slots->ring1), NULL, s_pos_ring1, COLOR_WHITE);

        if (slots->ring2 != EQM_NULL)
            draw_sprite(EQUIPMENT_ICON(slots->ring2), NULL, s_pos_ring2, COLOR_WHITE);
    }
    end_sprite();

    vec4 text_color = COLOR_RED_INIT;
    begin_text(NULL, projection_matrix);
    {
        if (slots->weapon != EQM_NULL)
            draw_textv_ex(EQUIPMENT_NAME(slots->weapon),
                          s_font_8px,
                          s_pos_weapon,
                          1.f,
                          TEXT_ALIGN_LEFT,
                          text_color);

        if (slots->armor != EQM_NULL)
            draw_textv_ex(EQUIPMENT_NAME(slots->armor),
                          s_font_8px,
                          s_pos_armor,
                          1.f,
                          TEXT_ALIGN_LEFT,
                          text_color);

        if (slots->boots != EQM_NULL)
            draw_textv_ex(EQUIPMENT_NAME(slots->boots),
                          s_font_8px,
                          s_pos_boots,
                          1.f,
                          TEXT_ALIGN_LEFT,
                          text_color);

        if (slots->ring1 != EQM_NULL)
            draw_textv_ex(EQUIPMENT_NAME(slots->ring1),
                          s_font_8px,
                          s_pos_ring1,
                          1.f,
                          TEXT_ALIGN_LEFT,
                          text_color);

        if (slots->ring2 != EQM_NULL)
            draw_textv_ex(EQUIPMENT_NAME(slots->ring2),
                          s_font_8px,
                          s_pos_ring2,
                          1.f,
                          TEXT_ALIGN_LEFT,
                          text_color);
    }
    end_text();
}
