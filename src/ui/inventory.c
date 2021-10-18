#include "ui/inventory.h"
#include "equipment.h"
#include "equipment_system.h"
#include "graphics.h"
#include "input.h"
#include "inventory.h"
#include "item.h"
#include "scn_main.h"
#include "string_utils.h"
#include "ui/equipment.h"
#include "ui/menu.h"

#define CELL_SIZE 16
#define CELL_GAP 9
#define INV_X 60
#define INV_Y 25
#define NUM_COLS 7
#define NUM_ROWS 3
#define MAX_CELLS 21
#define FIRST_CELL_X (INV_X + 17)
#define FIRST_CELL_Y (INV_Y + 37)
#define ITEM_NAME_X (INV_X + 100)
#define ITEM_NAME_Y (INV_Y + 14)
#define CURSOR_WIDTH 26
#define CURSOR_HEIGHT 26
#define CURSOR_FRAME_DURATION 10
#define CURSOR_NUM_FRAMES 6

#define ITEM_DESCRIPTION_X (INV_X + 27)
#define ITEM_DESCRIPTION_Y (INV_Y + 120)
#define WINDOW_SCALE (4)

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
static ecs_Registry* s_registry;

static vec2 s_icon_pos[ITEM_CATEGORY_CNT] = {
    { INV_X + 12, INV_Y + 5 },
    { INV_X + 37, INV_Y + 5 },
    { INV_X + 147, INV_Y + 5 },
    { INV_X + 172, INV_Y + 5 },
};
static vec2 s_active_tab_pos[] = {
    { INV_X + 10, INV_Y - 1 },
    { INV_X + 35, INV_Y - 1 },
    { INV_X + 145, INV_Y - 1 },
    { INV_X + 170, INV_Y - 1 },
};

static vec4 s_text_color = COLOR_INIT(0x4a, 0x3d, 0x3c, 0xff);

#define DESC_MAX_LEN ((sizeof s_description) - 1)

int
ui_inventory_init(const Resources* resources, ecs_Registry* registry)
{
    s_registry = registry;
    s_font_8px = &resources->font_8px;

    /* a small shortcut */
    const SpriteSheet* sheet = &resources->spritesheet;

    if ((s_sprite_cursor = sprite_sheet_get(sheet, "ui_inventory_cursor.png")) == NULL)
        LEAVE_ERROR(-1, "cursor sprite not found\n");

    if ((s_sprite_inventory = sprite_sheet_get(sheet, "ui_inventory.png")) == NULL)
        LEAVE_ERROR(-1, "inventory sprite not found\n");

    if ((s_sprite_consumable_icon = sprite_sheet_get(sheet, "icon_consumable.png")) == NULL)
        LEAVE_ERROR(-1, "consumable_icon sprite not found\n");

    if ((s_sprite_equipment_icon = sprite_sheet_get(sheet, "icon_weapon.png")) == NULL)
        LEAVE_ERROR(-1, "equipment_icon sprite not found\n");

    if ((s_sprite_spell_icon = sprite_sheet_get(sheet, "icon_spell.png")) == NULL)
        LEAVE_ERROR(-1, "spell_icon sprite not found\n");

    if ((s_sprite_key_ivon = sprite_sheet_get(sheet, "icon_key.png")) == NULL)
        LEAVE_ERROR(-1, "key_icon sprite not found\n");

    if ((s_sprite_active_tab = sprite_sheet_get(sheet, "ui_inventory_active_tab.png")) == NULL)
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
    scene_main_resume();
}

static const char s_option_use[]        = "Use";
static const char s_option_drop[]       = "Drop";
static const char s_option_equip[]      = "Equip";
static const char s_option_ring_slot1[] = "Ring 1";
static const char s_option_ring_slot2[] = "Ring 2";

static const char* s_options_consumable[] = {
    s_option_use,
    s_option_drop,
    NULL,
};

static const char* s_options_equipment[] = {
    s_option_equip,
    s_option_drop,
    NULL,
};

static const char* s_options_key[] = {
    s_option_use,
    s_option_drop,
    NULL,
};

static const char* s_options_spell[] = {
    s_option_use,
    s_option_drop,
    NULL,
};

static const char* s_options_equip_ring[] = {
    s_option_ring_slot1,
    s_option_ring_slot2,
    NULL,
};

static void handle_consumable_option(void*, const char*, u32);
static void handle_equipment_option(void*, const char*, u32);
static void handle_spell_option(void*, const char*, u32);
static void handle_key_option(void*, const char*, u32);
static void handle_equip_ring_option(void*, const char*, u32);

static void
handle_use_item(void)
{
    struct ItemSlot slot = inv_get_slot((enum ItemCategory)s_active_tab, s_cur_y, s_cur_x);

    if (slot.quantity)
    {
        switch (s_active_tab)
        {
        case ITEM_CATEGORY_CONSUMABLE:
            ui_menu_set_options(s_options_consumable);
            ui_menu_callback(handle_consumable_option, NULL);
            break;
        case ITEM_CATEGORY_EQUIPMENT:
            ui_menu_set_options(s_options_equipment);
            ui_menu_callback(handle_equipment_option, NULL);
            break;
        case ITEM_CATEGORY_SPELL:
            ui_menu_set_options(s_options_spell);
            ui_menu_callback(handle_spell_option, NULL);
            break;
        case ITEM_CATEGORY_KEY:
            ui_menu_set_options(s_options_key);
            ui_menu_callback(handle_key_option, NULL);
            break;
        default:
            ASSERT(0);
        }

        ui_menu_display();
    }
}

static void
handle_consumable_option(UNUSED void* ctx, const char* option, u32 index)
{
}

static void
handle_equipment_option(UNUSED void* ctx, const char* option, u32 index)
{
    const struct ItemSlot slot = inv_get_slot(ITEM_CATEGORY_EQUIPMENT, s_cur_y, s_cur_x);
    if (g_equipments[g_items[slot.item_id].data.equipment.id].type == EQM_TYPE_RING)
    {
        ui_menu_set_options(s_options_equip_ring);
        ui_menu_callback(handle_equip_ring_option, NULL);
        ui_menu_display();
    }
    else
    {
        switch (g_equipments[g_items[slot.item_id].data.equipment.id].type)
        {
        case EQM_TYPE_ARMOR:
            eqm_equip_armor(s_registry, g_main_ctx.player, g_items[slot.item_id].data.equipment.id);
            break;
        case EQM_TYPE_BOOTS:
            eqm_equip_boots(s_registry, g_main_ctx.player, g_items[slot.item_id].data.equipment.id);
            break;
        }
        ui_equipment_show();
    }
}

static void
handle_spell_option(UNUSED void* ctx, const char* option, u32 index)
{
}

static void
handle_key_option(UNUSED void* ctx, const char* option, u32 index)
{
}

static void
handle_equip_ring_option(void* ctx, const char* option, u32 index)
{
    const struct ItemSlot slot = inv_get_slot(ITEM_CATEGORY_EQUIPMENT, s_cur_y, s_cur_x);
    if (index == 0)
        eqm_equip_ring1(s_registry, g_main_ctx.player, g_items[slot.item_id].data.equipment.id);
    else
        eqm_equip_ring2(s_registry, g_main_ctx.player, g_items[slot.item_id].data.equipment.id);
}

static void
process_input(UNUSED void* unused)
{
    if (button_just_pressed(BTN_CANCEL))
    {
        close();
    }

    if (button_just_pressed(BTN_INTERACT))
    {
        handle_use_item();
    }

    if (button_just_pressed(BTN_LEFT))
        s_cur_x = max(s_cur_x - 1, 0);

    if (button_just_pressed(BTN_RIGHT))
        s_cur_x = min(s_cur_x + 1, INVENTORY_COLUMN_CNT - 1);

    if (button_just_pressed(BTN_UP))
        s_cur_y = max(s_cur_y - 1, 0);

    if (button_just_pressed(BTN_DOWN))
        s_cur_y = min(s_cur_y + 1, INVENTORY_ROW_CNT - 1);

    if (mouse_button_just_pressed(SDL_BUTTON_LEFT))
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_Point p = { mx / WINDOW_SCALE, my / WINDOW_SCALE };

        for (int i = 0; i < ITEM_CATEGORY_CNT; ++i)
        {
            SDL_Rect r;
            r.x = (int)s_active_tab_pos[i][0];
            r.y = (int)s_active_tab_pos[i][1];
            r.w = 20.f;
            r.h = 20.f;
            if (SDL_PointInRect(&p, &r))
            {
                s_active_tab = i;
                break;
            }
        }
    }
}

void
ui_inventory_show()
{
    s_visible = true;
    input_push(INPUT_CALLBACK1(process_input));
    scene_main_pause();
}

static void
draw_equipment_description(const struct Item* item)
{
    char strbuf_target[256];

    struct StrBuf strbuf = { .data = strbuf_target, .allocated = sizeof(strbuf_target), .used = 0 };

    static char type_string[EQM_TYPE_CNT][64] = { [EQM_TYPE_WEAPON] = "Weapon",
                                                  [EQM_TYPE_ARMOR]  = "Armor",
                                                  [EQM_TYPE_BOOTS]  = "Boots",
                                                  [EQM_TYPE_RING]   = "Ring" };

    const struct Equipment* equipment = &g_equipments[item->data.equipment.id];

    strbuf_append_fmt(&strbuf, "Type: %s\n", type_string[equipment->type]);
    strbuf_append_fmt(&strbuf, "%s\n", item->description);

    if (equipment->modifier.base.strength != 0)
        strbuf_append_fmt(&strbuf, "+%d str\n", (int)equipment->modifier.base.strength);

    if (equipment->modifier.base.intelligent != 0)
        strbuf_append_fmt(&strbuf, "+%d int\n", (int)equipment->modifier.base.intelligent);

    if (equipment->modifier.base.vitality != 0)
        strbuf_append_fmt(&strbuf, "+%d vit\n", (int)equipment->modifier.base.vitality);

    if (equipment->modifier.base.luck != 0)
        strbuf_append_fmt(&strbuf, "+%d lck\n", (int)equipment->modifier.base.luck);

    if (equipment->modifier.health != 0)
        strbuf_append_fmt(&strbuf, "+%d hp\n", (int)equipment->modifier.health);

    if (equipment->modifier.mana != 0)
        strbuf_append_fmt(&strbuf, "+%d mp\n", (int)equipment->modifier.mana);

    if (equipment->modifier.defense.physical != 0)
        strbuf_append_fmt(&strbuf, "+%d def\n", (int)equipment->modifier.defense.physical);

    if (equipment->modifier.defense.fire != 0)
        strbuf_append_fmt(&strbuf, "+%d fire def\n", (int)equipment->modifier.defense.fire);

    if (equipment->modifier.defense.ice != 0)
        strbuf_append_fmt(&strbuf, "+%d ice def\n", (int)equipment->modifier.defense.ice);

    if (equipment->modifier.defense.lighting != 0)
        strbuf_append_fmt(&strbuf, "+%d lighting def", (int)equipment->modifier.defense.lighting);

    if (equipment->modifier.defense.holy != 0)
        strbuf_append_fmt(&strbuf, "+%d holy def", (int)equipment->modifier.defense.holy);

    if (equipment->modifier.defense.dark != 0)
        strbuf_append_fmt(&strbuf, "+%d dark def", (int)equipment->modifier.defense.dark);

    draw_text_ex(strbuf_target,
                 s_font_8px,
                 ITEM_DESCRIPTION_X,
                 ITEM_DESCRIPTION_Y,
                 1.f,
                 TEXT_ALIGN_LEFT,
                 s_text_color);
}

static void
draw_consumable_description(const struct Item* item)
{
    draw_text_ex(item->description,
                 s_font_8px,
                 ITEM_DESCRIPTION_X,
                 ITEM_DESCRIPTION_Y,
                 1.f,
                 TEXT_ALIGN_LEFT,
                 s_text_color);
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
    for (int i = 0; i < INVENTORY_ROW_CNT; ++i, pos[0] = FIRST_CELL_X, pos[1] += CELL_SIZE + CELL_GAP)
    {
        for (int j = 0; j < INVENTORY_COLUMN_CNT; ++j, pos[0] += CELL_SIZE + CELL_GAP)
        {
            struct ItemSlot slot = inv_get_slot((enum ItemCategory)s_active_tab, i, j);
            if (slot.quantity != 0)
            {
                icon = item_icon(slot.item_id);
                if (slot.quantity > 1)
                {
                    sprintf(strbuf, "%d", slot.quantity);
                    draw_textv_ex(strbuf, s_font_8px, pos, 1.f, TEXT_ALIGN_LEFT, s_text_color);
                }
                draw_sprite(icon, NULL, pos, COLOR_WHITE);
            }
        }
    }

    pos[0] = (float)(FIRST_CELL_X + s_cur_x * (CELL_GAP + CELL_SIZE) + (CELL_SIZE / 2 - CURSOR_WIDTH / 2));
    pos[1] = (float)(FIRST_CELL_Y + s_cur_y * (CELL_GAP + CELL_SIZE) + (CELL_SIZE / 2 - CURSOR_HEIGHT / 2));
    const Sprite* frame = animation_get_frame(&s_cur_anim, s_elapsed_time);
    draw_sprite(frame, NULL, pos, COLOR_WHITE);

    struct ItemSlot selected_slot = inv_get_slot((enum ItemCategory)s_active_tab, s_cur_y, s_cur_x);
    if (selected_slot.quantity > 0)
    {
        draw_text_ex(item_name(selected_slot.item_id),
                     s_font_8px,
                     ITEM_NAME_X,
                     ITEM_NAME_Y,
                     1.f,
                     TEXT_ALIGN_CENTER,
                     s_text_color);

        switch (item_category(selected_slot.item_id))
        {
        case ITEM_CATEGORY_CONSUMABLE: /* fallthrough */
        case ITEM_CATEGORY_SPELL:
        case ITEM_CATEGORY_KEY:
            draw_consumable_description(&g_items[selected_slot.item_id]);
            break;
        case ITEM_CATEGORY_EQUIPMENT:
            draw_equipment_description(&g_items[selected_slot.item_id]);
            break;
        }
    }

    draw_sprite(s_sprite_active_tab, NULL, s_active_tab_pos[s_active_tab], COLOR_WHITE);
    draw_sprite(s_sprite_consumable_icon, NULL, s_icon_pos[ITEM_CATEGORY_CONSUMABLE], COLOR_WHITE);
    draw_sprite(s_sprite_equipment_icon, NULL, s_icon_pos[ITEM_CATEGORY_EQUIPMENT], COLOR_WHITE);
    draw_sprite(s_sprite_spell_icon, NULL, s_icon_pos[ITEM_CATEGORY_SPELL], COLOR_WHITE);
    draw_sprite(s_sprite_key_ivon, NULL, s_icon_pos[ITEM_CATEGORY_KEY], COLOR_WHITE);
}
