#include "input.h"
#include "toolbox.h"
#include <string.h>

static const int sBtnToScancodeTbl[BTN_CNT] = {
  [BTN_UP] = SDL_SCANCODE_W,        [BTN_DOWN] = SDL_SCANCODE_S,
  [BTN_LEFT] = SDL_SCANCODE_A,      [BTN_RIGHT] = SDL_SCANCODE_D,
  [BTN_INTERACT] = SDL_SCANCODE_E,  [BTN_CANCEL] = SDL_SCANCODE_Q,
  [BTN_MENU] = SDL_SCANCODE_ESCAPE, [BTN_JUMP] = SDL_SCANCODE_SPACE,
  [BTN_INVENTORY] = SDL_SCANCODE_I,
};

static u32 sCurrMouseBtnState;
static u32 sPrevMouseBtnState;
static u32 sCurrButtonState;
static u32 sPrevButtonState;

static u8 sCurrKeyState[SDL_NUM_SCANCODES] = { 0 };
static u8 sPrevKeyState[SDL_NUM_SCANCODES] = { 0 };

#define STACK_SIZE 10

static InputCallback sCallbackStack[STACK_SIZE];
static u32           sCallbackStackCnt;

void
input_init()
{
}

static void
update_key_state(void)
{
  SDL_memcpy(sPrevKeyState, sCurrKeyState, sizeof(sPrevKeyState));
  SDL_memcpy(sCurrKeyState, SDL_GetKeyboardState(NULL), sizeof(sCurrKeyState));
}

static void
update_button_state(void)
{
  const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

  sPrevButtonState = sCurrButtonState;
  sCurrButtonState = 0;

  for (int i = 0; i < BTN_CNT; ++i)
    sCurrButtonState |= (keyboard_state[sBtnToScancodeTbl[i]] & 1) << i;
}

void
input_update()
{
  update_key_state();
  update_button_state();
  update_mouse_state();

  InputCallback cb;
  if (sCallbackStackCnt > 0)
  {
    cb = sCallbackStack[sCallbackStackCnt - 1];
    cb.fn(cb.data, sCurrButtonState, sPrevButtonState);
  }
}

SDL_bool
button_pressed(u16 btn)
{
  return sCurrButtonState & BUTTON_MASK(btn);
}

SDL_bool
button_just_pressed(u16 btn)
{
  return (sCurrButtonState & BUTTON_MASK(btn)) &&
         !(sPrevButtonState & BUTTON_MASK(btn));
}

void
input_push(InputCallback inputCallback)
{
  ASSERT(sCallbackStackCnt < STACK_SIZE);
  sCallbackStack[sCallbackStackCnt++] = inputCallback;
}

void
input_pop()
{
  if (sCallbackStackCnt > 0)
    sCallbackStackCnt--;
}

SDL_Scancode
button_to_scancode(u16 btn)
{
  ASSERT(btn < BTN_CNT);
  return sBtnToScancodeTbl[btn];
}

void
update_mouse_state()
{
  sPrevMouseBtnState = sCurrMouseBtnState;
  sCurrMouseBtnState = SDL_GetMouseState(NULL, NULL);
}

BOOL
mouse_button_just_pressed(u16 msBtn)
{
  return !(sPrevMouseBtnState & SDL_BUTTON(msBtn)) &&
         (sCurrMouseBtnState & SDL_BUTTON(msBtn));
}

BOOL
mouse_button_pressed(u16 msBtn)
{
  return sCurrMouseBtnState & SDL_BUTTON(msBtn);
}

BOOL
key_just_pressed(SDL_Scancode scancode)
{
  return sCurrKeyState[scancode] && !sPrevKeyState[scancode];
}

BOOL
key_just_released(SDL_Scancode scancode)
{
  return !sCurrKeyState[scancode] && sPrevKeyState[scancode];
}

BOOL
key_pressed(SDL_Scancode scancode)
{
  return sCurrKeyState[scancode];
}

BOOL
key_released(SDL_Scancode scancode)
{
  return !sCurrKeyState[scancode];
}
