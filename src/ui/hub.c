#include "ui/hud.h"

static const SDL_Rect _start = { 0, 0, 4, 9 };

static const SDL_Rect _body = { 4, 0, 3, 9 };

static const SDL_Rect _end = { 7, 0, 5, 9 };

static const SDL_Color blood_active = { 237, 19, 41, 255 };

static const SDL_Color blood_inactive = { 135, 11, 23, 255 };

static const SDL_Color mana_active = { 87, 152, 203, 255 };

static const SDL_Color mana_inactive = { 96, 47, 86, 255 };

typedef struct DrawBarParams
{
    u16   length;
    u16   points;
    float position_x;
    float position_y;
    vec4  active_color;
    vec4  inactive_color;
} DrawBarParams;

static Sprite      s_sprite_start;
static Sprite      s_sprite_end;
static Sprite      s_sprite_mid;
static const Font* s_font_8px;

int
ui_hub_init(const SpriteSheet* spriteshet, const Font* font_8px)
{
    const Sprite* bar_sprite = sprite_sheet_get(spriteshet, "player_health_bar.png");
    if (bar_sprite == NULL)
        LEAVE_ERROR(-1, "player_health_bar.png not fonud\n");

    return 0;
}

void
draw_bar(const DrawBarParams* params)
{
    SDL_Rect dst;

    vec2 pos = { params->position_x, params->position_y };
    draw_sprite(&s_sprite_start, NULL, pos, COLOR_WHITE);

    const float start_width = (float)s_sprite_start.rect.w;
    const float mid_width = (float)s_sprite_mid.rect.w;
    pos[0] += start_width; 
    for (int i = 0; i < params->length; ++i, pos[0] += mid_width)
        draw_sprite(&s_sprite_mid, NULL, pos, COLOR_WHITE);

    pos[0] = params->position_x + start_width + params->length * mid_width;
    pos[0] = params->position_y;
    draw_sprite(&s_sprite_end, NULL, pos, COLOR_WHITE);

    Sprite sprite;
    sprite.texture = NULL;
    sprite.rect.w = 2;
    sprite.rect.h = 3;
    for (int i = 0; i < params->length; ++i)
    {
        dst.x = params->position_x + (_start.w + 1) + i * 3;
        dst.y = params->position_y + 3;
        dst.w = 2;
        dst.h = 3;
        if (i < params->points)
        {
            SDL_SetRenderDrawColor(g_renderer,
                                   params->active_color.r,
                                   params->active_color.g,
                                   params->active_color.b,
                                   params->active_color.a);
        }
        else
        {
            SDL_SetRenderDrawColor(g_renderer,
                                   params->inactive_color.r,
                                   params->inactive_color.g,
                                   params->inactive_color.b,
                                   params->inactive_color.a);
        }
        SDL_RenderFillRect(g_renderer, &dst);
    }
}
