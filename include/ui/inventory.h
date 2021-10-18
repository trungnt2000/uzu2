// ngotrung Sat 04 Sep 2021 05:43:17 PM +07
#ifndef UI_INVENTORY_H
#define UI_INVENTORY_H
#include "graphics.h"
#include "resources.h"
#include "ecs.h"
int  ui_inventory_init(const Resources* resources, ecs_Registry* registry);
void ui_inventory_shutdown(void);
void ui_inventory_show(void);
void ui_inventory_draw(float deltaTime);
#endif // UI_INVENTORY_H
