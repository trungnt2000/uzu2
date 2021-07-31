#ifndef GAMESCR_H
#define GAMESCR_H
#include "ecs.h"
#include "toolbox.h"

extern ecs_Registry* gRegistry;
extern ecs_entity_t gPlayer;
void scene_main_pause(void);
void scene_main_resume(void);
#endif // GAMESCR_H
