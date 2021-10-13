#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H
#include "ecs.h"

struct MainContext
{
    ecs_entity_t player;
    ecs_entity_t current_object;
};

void scene_main_pause(void);
void scene_main_resume(void);
#endif // SCENE_MAIN_H
