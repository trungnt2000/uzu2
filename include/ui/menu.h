// ngotrung Tue 12 Oct 2021 12:43:25 PM +07
#ifndef UI_MENU_H
#define UI_MENU_H
#include "cglm/cglm.h"
#include "toolbox.h"
#include "graphics.h"

void ui_menu_init(const Font* font8px);
void ui_menu_set_options(const char* const* options);
void ui_menu_set_position(float x, float y);
void ui_menu_display(void);
void ui_menu_draw(mat4 mvp_matrix);
void ui_menu_callback(void(void*, const char* opt, u32 index), void* ctx);
#endif // UI_MENU_H
