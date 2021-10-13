// ngotrung Mon 11 Oct 2021 10:07:27 PM +07
#ifndef UI_EQUIPMENT_H
#define UI_EQUIPMENT_H
#include "cglm/cglm.h"
#include "ecs.h"
#include "graphics.h"

void ui_equipment_init(const Font* font);
void ui_equipment_show(void);
void ui_equipment_draw(ecs_Registry* registry, ecs_entity_t player, mat4 projection_matrix);
#endif // UI_EQUIPMENT_H
