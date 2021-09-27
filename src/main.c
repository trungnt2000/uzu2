#include "main.h"
#include "SDL_image.h"
#include "constances.h"
#include "ecs.h"
#include "engine.h"
#include "gameconf.h"
#include "graphics/gl.h"
#include "input.h"
#include "map.h"
#include "toolbox.h"
#include <stdio.h>
#include <stdlib.h>

static SceneId s_current_scene = SCENE_UNDEFINED;

/* defined in src/scn_title.c */
extern void scene_title_tick(float);
extern void scene_title_enter(void);
extern void scene_title_receive_event(const SDL_Event* event);
extern void scene_title_leave(void);
extern int  scene_title_create(void);
extern void scene_title_destroy(void);

/* defined in src/scn_main.c */
extern void scene_main_tick(float);
extern void scene_main_enter(void);
extern void scene_main_receive_event(const SDL_Event* event);
extern void scene_main_leave(void);
extern int  scene_main_create(void);
extern void scene_main_destroy(void);

/* defined in src/scn_select_character.c */
extern void scene_select_character_tick(float);
extern void scene_select_character_enter(void);
extern void scene_select_character_receive_event(const SDL_Event* event);
extern void scene_select_character_leave(void);
extern int  scene_select_character_create(void);
extern void scene_select_character_destroy(void);

static void (*const s_scene_enter_func_tbl[])(void) = { scene_title_enter,
                                                        scene_main_enter,
                                                        scene_select_character_enter };

static void (*const s_scene_leave_func_tbl[])(void) = { scene_title_leave,
                                                        scene_main_leave,
                                                        scene_select_character_leave };

static void (*const s_scene_receive_event_func_tbl[])(const SDL_Event*) = {
    scene_title_receive_event,
    scene_main_receive_event,
    scene_select_character_receive_event
};

static void (*const s_scene_tick_func_tbl[])(float) = {
    scene_title_tick,
    scene_main_tick,
    scene_select_character_tick,
};

void
configure(GameConf* conf)
{
    conf->width  = WIN_WIDTH;
    conf->height = WIN_HEIGHT;
    conf->vsync  = true;
}

bool
create()
{
    if (scene_title_create() != 0)
        return false;

    if (scene_main_create() != 0)
        return false;

    if (scene_select_character_create() != 0)
        return false;

    set_scene(SCENE_MAIN);
    printf("game created\n");
    return true;
}

void
destroy()
{
    scene_main_destroy();
    scene_title_destroy();
    scene_select_character_destroy();
    printf("game destroyed!\n");
}
void
tick(float delta_time)
{
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (s_current_scene != SCENE_UNDEFINED)
        s_scene_tick_func_tbl[s_current_scene](delta_time);
}

void
receive_event(const SDL_Event* e)
{
    if (e->type == SDL_QUIT)
        engine_stop();
    if (s_current_scene != SCENE_UNDEFINED)
        s_scene_receive_event_func_tbl[s_current_scene](e);
}

void
set_scene(SceneId new_scene)
{
    if (s_current_scene == new_scene)
        return;
    if (s_current_scene != SCENE_UNDEFINED)
        s_scene_leave_func_tbl[s_current_scene]();
    if (new_scene != SCENE_UNDEFINED)
        s_scene_enter_func_tbl[new_scene]();
    s_current_scene = new_scene;
}
