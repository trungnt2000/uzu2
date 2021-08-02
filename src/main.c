#include "main.h"
#include "graphics/gl.h"
#include "SDL_image.h"
#include "constances.h"
#include "ecs.h"
#include "engine.h"
#include "input.h"
#include "map.h"
#include "toolbox.h"
#include <stdio.h>
#include <stdlib.h>

static SceneId sCurrentScene = SCENE_UNDEFINED;

/* defined in src/scn_title.c */
extern void scene_title_tick(float);
extern void scene_title_enter(void);
extern void scene_title_receive_event(const SDL_Event* event);
extern void scene_title_leave(void);
extern void scene_title_create(void);
extern void scene_title_destroy(void);

/* defined in src/scn_main.c */
extern void scene_main_tick(float);
extern void scene_main_enter(void);
extern void scene_main_receive_event(const SDL_Event* event);
extern void scene_main_leave(void);
extern void scene_main_create(void);
extern void scene_main_destroy(void);

/* defined in src/scn_select_character.c */
extern void scene_select_character_tick(float);
extern void scene_select_character_enter(void);
extern void scene_select_character_receive_event(const SDL_Event* event);
extern void scene_select_character_leave(void);
extern void scene_select_character_create(void);
extern void scene_select_character_destroy(void);

static void (*const sScnEnterFuncTbl[])(void) = {
  scene_title_enter,
  scene_main_enter,
  scene_select_character_enter
};

static void (*const sScnLeaveFuncTbl[])(void) = {
  scene_title_leave,
  scene_main_leave,
  scene_select_character_leave
};

static void (*const sScnReceiveEventFuncTbl[])(const SDL_Event*) = {
  scene_title_receive_event,
  scene_main_receive_event,
  scene_select_character_receive_event
};

static void (*const sScnTickFuncTbl[])(float) = {
  scene_title_tick,
  scene_main_tick,
  scene_select_character_tick,
};

BOOL
create()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  scene_title_create();
  scene_main_create();
  scene_select_character_create();
  set_scene(SCENE_MAIN);
  printf("game created\n");
  return UZU_TRUE;
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
tick(float deltaTime)
{
  glClearColor(.0f, .3f, .5f, .0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (sCurrentScene != SCENE_UNDEFINED)
    sScnTickFuncTbl[sCurrentScene](deltaTime);
}

void
receive_event(const SDL_Event* e)
{
  if (e->type == SDL_QUIT)
    engine_stop();
  if (sCurrentScene != SCENE_UNDEFINED)
    sScnReceiveEventFuncTbl[sCurrentScene](e);
}

void
set_scene(SceneId newScene)
{
  if (sCurrentScene == newScene)
    return;
  if (sCurrentScene != SCENE_UNDEFINED)
    sScnLeaveFuncTbl[sCurrentScene]();
  if (newScene != SCENE_UNDEFINED)
    sScnEnterFuncTbl[newScene]();
  sCurrentScene = newScene;
}
