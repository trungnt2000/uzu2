#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H
#include "ecs.h"
#include "graphics.h"

struct MainContext
{
    ecs_entity_t player;
    ecs_entity_t current_object;
};

extern struct MainContext g_main_ctx;

void scene_main_pause(void);
void scene_main_resume(void);

void request_load_level(const char* level_name);
#endif // SCENE_MAIN_H
