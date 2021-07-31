#include "constances.h"
#include "engine.h"
#include "input.h"

/*
static void main_state_process_input(void*);
static void about_state_process_input(void*);
static void draw_main_menu(void);
static void draw_about_panel(void);
*/

#define POS_X 117
#define POS_Y 78
#define NUM_OPTS 4

enum
{
  STATE_MAIN,
  STATE_ABOUT,
};

/*
static const char sTxtNewGame[] = "New Game";
static const char sTxtExit[]    = "Exit";
static const char sTxtAbout[]   = "About";
static const char sTxtLoad[]    = "Load";
static s8         sSelectedIdx;
static u8         sState;

static const char* const sOpts[] = {
  sTxtLoad,
  sTxtNewGame,
  sTxtAbout,
  sTxtExit,
};
*/
void scene_title_create()
{
}

void scene_title_destroy()
{
}

void scene_title_enter()
{
}

void scene_title_leave()
{
}

void scene_title_tick(float deltaTime)
{
}

void scene_title_receive_event(const SDL_UNUSED SDL_Event* e)
{
}

