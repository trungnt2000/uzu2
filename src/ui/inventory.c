#include "ui/inventory.h"
#include "graphics.h"
#include "input.h"
#include "inventory.h"

#define CELL_SIZE 16
#define CELL_GAP 9
#define INV_X 60
#define INV_Y 25
#define NUM_COLS 7
#define NUM_ROWS 3
#define MAX_CELLS 21
#define FIRST_CELL_X (INV_X + 17)
#define FIRST_CELL_Y (INV_Y + 37)
#define ITEM_NAME_X (INV_X + 97)
#define ITEM_NAME_Y (INV_Y + 14)
#define CURSOR_WIDTH 26
#define CURSOR_HEIGHT 26
#define CURSOR_FRAME_DURATION 10
#define CURSOR_NUM_FRAMES 6

#define ITEM_DESCRIPTION_X (INV_X + 27)
#define ITEM_DESCRIPTION_Y (INV_Y + 120)

static int       s_cur_x;
static int       s_cur_y;
static bool      s_visible;
static Animation s_cur_anim;
static float     s_elapsed_time;
static int       s_active_tab;
static vec2      s_position = { INV_X, INV_Y };

// resources
static const Sprite* s_sprite_inventory;
static const Sprite* s_sprite_cursor;
static const Sprite* s_sprite_consumable_icon;
static const Sprite* s_sprite_equipment_icon;
static const Sprite* s_sprite_spell_icon;
static const Sprite* s_sprite_key_ivon;
static const Sprite* s_sprite_active_tab;
static const Font*   s_font_8px;

static vec2 s_icon_dst_tbl[ITEM_CATEGORY_CNT] = {
    { INV_X + 12, INV_Y + 5 },
    { INV_X + 37, INV_Y + 5 },
    { INV_X + 147, INV_Y + 5 },
    { INV_X + 172, INV_Y + 5 },
};
static vec2 s_active_tab_dst_tbl[] = {
    { INV_X + 10, INV_Y - 1 },
    { INV_X + 35, INV_Y - 1 },
    { INV_X + 145, INV_Y - 1 },
    { INV_X + 170, INV_Y - 1 },
};

int
ui_inventory_init(const Resources* resources)
{
    s_font_8px = &resources->font_8px;

    /* a small shortcut */
    const SpriteSheet* sheet = &resources->spritesheet;

    if ((s_sprite_cursor = sprite_sheet_get(sheet, "inventory_cursor.png")) == NULL)
        LEAVE_ERROR(-1, "cursor sprite not found\n");

    if ((s_sprite_inventory = sprite_sheet_get(sheet, "inventory.png")) == NULL)
        LEAVE_ERROR(-1, "inventory sprite not found\n");

    if ((s_sprite_consumable_icon = sprite_sheet_get(sheet, "icon_consumable.png")) == NULL)
        LEAVE_ERROR(-1, "consumable_icon sprite not found\n");

    if ((s_sprite_equipment_icon = sprite_sheet_get(sheet, "icon_weapon.png")) == NULL)
        LEAVE_ERROR(-1, "equipment_icon sprite not found\n");

    if ((s_sprite_spell_icon = sprite_sheet_get(sheet, "icon_spell.png")) == NULL)
        LEAVE_ERROR(-1, "spell_icon sprite not found\n");

    if ((s_sprite_key_ivon = sprite_sheet_get(sheet, "key_item.png")) == NULL)
        LEAVE_ERROR(-1, "key_icon sprite not found\n");

    if ((s_sprite_active_tab = sprite_sheet_get(sheet, "inventory_tab_active.png")) == NULL)
        LEAVE_ERROR(-1, "inventory_active_tab sprite not found\n");

    AnimationTemplate tmpl = { 0 };
    tmpl.column_count      = 5;
    tmpl.row_count         = 1;
    tmpl.offset_x          = 0;
    tmpl.offset_y          = 0;
    tmpl.frame_duration    = 0.1f;
    tmpl.sprite_width      = 26;
    tmpl.sprite_height     = 26;
    animation_init_w_sprite(&s_cur_anim, s_sprite_cursor, &tmpl);

    return 0;
}

void
ui_inventory_shutdown()
{
    s_sprite_inventory       = NULL;
    s_sprite_cursor          = NULL;
    s_sprite_consumable_icon = NULL;
    s_sprite_equipment_icon  = NULL;
    s_sprite_spell_icon      = NULL;
    s_sprite_key_ivon        = NULL;
    s_sprite_active_tab      = NULL;

    s_font_8px = NULL;
}

static void
close()
{
    s_visible = false;
    input_pop();
}

static void
process_input(void* unused, u32 current_state, u32 previous_state)
{
    if (button_just_pressed(BTN_CANCEL))
    {
        close();
    }

    if (button_just_pressed(BTN_LEFT))
        s_cur_x = max(s_cur_x - 1, 0);

    if (button_just_pressed(BTN_RIGHT))
        s_cur_x = min(s_cur_x + 1, INVENTORY_COLUMN_CNT - 1);

    if (button_just_pressed(BTN_UP))
        s_cur_y = max(s_cur_y - 1, 0);

    if (button_just_pressed(BTN_DOWN))
        s_cur_y = min(s_cur_y + 1, INVENTORY_ROW_CNT - 1);
}

void
ui_inventory_show()
{
    s_visible = true;
    input_push(INPUT_CALLBACK1(process_input));
}

void
ui_inventory_draw(float delta_time)
{
    if (!s_visible)
        return;

    s_elapsed_time += delta_time;
    draw_sprite(s_sprite_inventory, NULL, s_position, COLOR_WHITE);

    vec2          pos = { FIRST_CELL_X, FIRST_CELL_Y };
    const Sprite* icon;
    char          strbuf[16];
    vec4          text_color = COLOR_INIT(0x4a, 0x3d, 0x3c, 0xff);
    for (int i = 0; i < INVENTORY_ROW_CNT; ++i, pos[0] = FIRST_CELL_X, pos[1] += CELL_SIZE + CELL_GAP)
    {
        for (int j = 0; j < INVENTORY_ROW_CNT; ++j, pos[0] += CELL_SIZE + CELL_GAP)
        {
            ItemSlot slot = inventory_get_slot((ItemCategory)s_active_tab, i, j);
            if (slot.quality != 0)
            {
                icon = item_icon(slot.item_id);
                sprintf(strbuf, "%d", slot.quality);
                draw_sprite(icon, NULL, pos, COLOR_WHITE);
                // draw_text(strbuf, pos[0], pos[1], text_color);
                draw_textv_ex(strbuf, s_font_8px, pos, 1.f, TEXT_ALIGN_LEFT, text_color);
            }
        }
    }
    pos[0] = (float)(FIRST_CELL_X + s_cur_x * (CELL_GAP + CELL_SIZE) + (CELL_SIZE / 2 - CURSOR_WIDTH / 2));
    pos[1] = (float)(FIRST_CELL_Y + s_cur_y * (CELL_GAP + CELL_SIZE) + (CELL_SIZE / 2 - CURSOR_HEIGHT / 2));
    const Sprite* frame = animation_get_frame(&s_cur_anim, s_elapsed_time);
    draw_sprite(frame, NULL, pos, COLOR_WHITE);

    ItemSlot selected_slot = inventory_get_slot((ItemCategory)s_active_tab, s_cur_y, s_cur_x);
    if (selected_slot.quality > 0)
    {
        draw_text_ex(item_name(selected_slot.item_id),
                     s_font_8px,
                     ITEM_NAME_X,
                     ITEM_NAME_Y,
                     1.f,
                     TEXT_ALIGN_CENTER,
                     text_color);

        draw_text_ex(item_description(selected_slot.item_id),
                     s_font_8px,
                     ITEM_DESCRIPTION_X,
                     ITEM_DESCRIPTION_Y,
                     1.f,
                     TEXT_ALIGN_LEFT,
                     text_color);
    }

    draw_sprite(s_sprite_active_tab, NULL, s_active_tab_dst_tbl[s_active_tab], COLOR_WHITE);
    draw_sprite(s_sprite_consumable_icon, NULL, s_icon_dst_tbl[ITEM_CATEGORY_CONSUMABLE], COLOR_WHITE);
    draw_sprite(s_sprite_equipment_icon, NULL, s_icon_dst_tbl[ITEM_CATEGORY_EQUIPMENT], COLOR_WHITE);
    draw_sprite(s_sprite_spell_icon, NULL, s_icon_dst_tbl[ITEM_CATEGORY_SPELL], COLOR_WHITE);
    draw_sprite(s_sprite_key_ivon, NULL, s_icon_dst_tbl[ITEM_CATEGORY_KEY], COLOR_WHITE);
}
