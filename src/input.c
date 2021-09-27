#include "input.h"
#include "toolbox.h"
#include <string.h>

static const int s_tbn_to_scancode[BTN_CNT] = {
    [BTN_UP] = SDL_SCANCODE_W,        [BTN_DOWN] = SDL_SCANCODE_S,     [BTN_LEFT] = SDL_SCANCODE_A,
    [BTN_RIGHT] = SDL_SCANCODE_D,     [BTN_INTERACT] = SDL_SCANCODE_E, [BTN_CANCEL] = SDL_SCANCODE_Q,
    [BTN_MENU] = SDL_SCANCODE_ESCAPE, [BTN_JUMP] = SDL_SCANCODE_SPACE, [BTN_INVENTORY] = SDL_SCANCODE_I,
};

static u32 s_curr_mouse_state;
static u32 s_prev_mouse_state;
static u32 s_curr_button_state;
static u32 s_prev_button_state;

static u8 s_curr_key_state[SDL_NUM_SCANCODES] = { 0 };
static u8 s_prev_key_state[SDL_NUM_SCANCODES] = { 0 };

#define STACK_SIZE 10

static InputCallback s_callback_stack[STACK_SIZE];
static u32           s_callback_stack_cnt;

void
input_init()
{
}

static void
update_key_state(void)
{
    SDL_memcpy(s_prev_key_state, s_curr_key_state, sizeof(s_prev_key_state));
    SDL_memcpy(s_curr_key_state, SDL_GetKeyboardState(NULL), sizeof(s_curr_key_state));
}

static void
update_button_state(void)
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

    s_prev_button_state = s_curr_button_state;
    s_curr_button_state = 0;

    for (int i = 0; i < BTN_CNT; ++i)
        s_curr_button_state |= (keyboard_state[s_tbn_to_scancode[i]] & 1) << i;
}

void
input_update()
{
    update_key_state();
    update_button_state();
    update_mouse_state();

    InputCallback cb;
    if (s_callback_stack_cnt > 0)
    {
        cb = s_callback_stack[s_callback_stack_cnt - 1];
        cb.fn(cb.data, s_curr_button_state, s_prev_button_state);
    }
}

bool
button_pressed(u16 btn)
{
    return s_curr_button_state & BUTTON_MASK(btn);
}

bool
button_just_pressed(u16 btn)
{
    return (s_curr_button_state & BUTTON_MASK(btn)) && !(s_prev_button_state & BUTTON_MASK(btn));
}

void
input_push(InputCallback cb)
{
    ASSERT(s_callback_stack_cnt < STACK_SIZE);
    s_callback_stack[s_callback_stack_cnt++] = cb;
}

void
input_pop()
{
    if (s_callback_stack_cnt > 0)
        s_callback_stack_cnt--;
}

SDL_Scancode
button_to_scancode(u16 btn)
{
    ASSERT(btn < BTN_CNT);
    return s_tbn_to_scancode[btn];
}

void
update_mouse_state()
{
    s_prev_mouse_state = s_curr_mouse_state;
    s_curr_mouse_state = SDL_GetMouseState(NULL, NULL);
}

bool
mouse_button_just_pressed(u16 msBtn)
{
    return !(s_prev_mouse_state & SDL_BUTTON(msBtn)) && (s_curr_mouse_state & SDL_BUTTON(msBtn));
}

bool
mouse_button_pressed(u16 msBtn)
{
    return s_curr_mouse_state & SDL_BUTTON(msBtn);
}

bool
key_just_pressed(SDL_Scancode scancode)
{
    return s_curr_key_state[scancode] && !s_prev_key_state[scancode];
}

bool
key_just_released(SDL_Scancode scancode)
{
    return !s_curr_key_state[scancode] && s_prev_key_state[scancode];
}

bool
key_pressed(SDL_Scancode scancode)
{
    return s_curr_key_state[scancode];
}

bool
key_released(SDL_Scancode scancode)
{
    return !s_curr_key_state[scancode];
}
