#include "ui/menu.h"
#include "graphics.h"
#include "input.h"

static void (*s_callback)(void* ctx, const char* opt, u32 index);
static void* s_callback_ctx;
static bool  s_visible;
static u32   s_selected_index;
static float s_position_x = 320 / 2;
static float s_position_y = 240 / 2;

#define NUM_OPTION_MAX 5
#define OPTION_LEN_MAX 63
#define MARGIN_TOP 5
#define MARGIN_LEFT 3
#define PADDING 3

static char s_options[NUM_OPTION_MAX][OPTION_LEN_MAX + 1];
static u32  s_num_options;

static const Font* s_font_8px;

static vec4 s_selected_color = COLOR_RED_INIT;
static vec4 s_option_color   = COLOR_BLUE_INIT;

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

    if (button_just_pressed(BTN_INTERACT))
    {
        close();
        s_callback(s_callback_ctx, s_options[s_selected_index], s_selected_index);
    }

    if (button_just_pressed(BTN_UP))
    {
        if (s_selected_index > 0)
            --s_selected_index;
    }

    if (button_just_pressed(BTN_DOWN))
    {
        if (s_selected_index < s_num_options - 1)
            ++s_selected_index;
    }
}

void
ui_menu_init(const Font* font8px)
{
    s_font_8px = font8px;
}

void
ui_menu_set_options(const char* const* options)
{
    s_num_options = 0;
    for (u32 i = 0; i < NUM_OPTION_MAX && options[i] != NULL; ++i)
    {
        s_num_options++;
        SDL_strlcpy(s_options[i], options[i], OPTION_LEN_MAX);
    }
    s_selected_index = 0;
}

void
ui_menu_display(void)
{
    s_visible = true;
    input_push(INPUT_CALLBACK1(process_input));
}

void
ui_menu_draw(mat4 mvp_matrix)
{
    if (!s_visible)
        return;
    begin_sprite(mvp_matrix);
    {
        struct Sprite sprite = { 0 };

        sprite.rect.x = (int)0;
        sprite.rect.y = (int)0;
        sprite.rect.h = (int)(s_num_options * (8 + PADDING));
        sprite.rect.w = 64;
        draw_sprite(&sprite, NULL, (vec2){ s_position_x, s_position_y }, COLOR_WHITE);
    }
    end_sprite();

    vec4 text_color;
    begin_text(NULL, mvp_matrix);
    for (u32 i = 0; i < s_num_options; ++i)
    {
        if (i == s_selected_index)
            glm_vec4_copy(s_selected_color, text_color);
        else
            glm_vec4_copy(s_option_color, text_color);

        float x = s_position_x + MARGIN_LEFT;
        float y = s_position_y + (float)i * (8.f + PADDING);
        draw_text_ex(s_options[i], s_font_8px, x, y, 1.f, TEXT_ALIGN_LEFT, text_color);
    }
    end_text();
}

void
ui_menu_callback(void (*cb)(void*, const char* opt, u32 index), void* ctx)
{
    s_callback     = cb;
    s_callback_ctx = ctx;
}
