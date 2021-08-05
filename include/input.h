#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "toolbox.h"
#include "SDL.h"

enum Button
{
  BTN_INTERACT,
  BTN_CANCEL,
  BTN_JUMP,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_UP,
  BTN_DOWN,
  BTN_MENU,
  BTN_INVENTORY,
  BTN_CNT
};

#define BUTTON_MASK(X) (1 << (X))

#define BTN_MSK_INTERACT BUTTON_MASK(BTN_INTERACT)
#define BTN_MSK_CANCEL BUTTON_MASK(BTN_CANCEL)
#define BTN_MSK_JUMP BUTTON_MASK(BTN_JUMP)
#define BTN_MSK_LEFT BUTTON_MASK(BTN_LEFT)
#define BTN_MSK_RIGHT BUTTON_MASK(BTN_RIGHT)
#define BTN_MSK_UP BUTTON_MASK(BTN_UP)
#define BTN_MSK_DOWN BUTTON_MASK(BTN_DOWN)
#define BTN_MSK_MENU BUTTON_MASK(BTN_MENU)
#define BTN_MSK_INVENTORY BUTTON_MASK(BTN_INVENTORY)

typedef struct InputCallback
{
  void (*fn)(void* userData, u32 currState, u32 prevState);
  void* data;
} InputCallback;

#define INPUT_CALLBACK1(__fn)                                                  \
  (InputCallback)                                                              \
  {                                                                            \
    .fn = (__fn), .data = NULL                                                 \
  }

#define INPUT_CALLBACK2(__fn, __data)                                          \
  (InputCallback)                                                              \
  {                                                                            \
    .fn = (__fn), .data = (__data)                                             \
  }

void         input_init(void);
void         input_update(void);
void         input_push(InputCallback inputCallback);
void         input_pop(void);
bool         button_pressed(u16 button);
bool         button_just_pressed(u16 button);
SDL_Scancode button_to_scancode(u16 button);
void         update_mouse_state(void);
bool         mouse_button_just_pressed(u16 button);
bool         mouse_button_pressed(u16 button);
bool         mouse_button_repeated(u16 button);
bool         key_pressed(SDL_Scancode scancode);
bool         key_just_pressed(SDL_Scancode scancode);
bool         key_released(SDL_Scancode scancode);
bool         key_just_released(SDL_Scancode scancode);
#endif
