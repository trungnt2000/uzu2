// ngotrung Thu 15 Jul 2021 09:17:19 PM +07
#ifndef VIEW_H
#define VIEW_H
#include "SDL.h"
#include "cglm/cglm.h"
#include "toolbox.h"

/* get combined matrix */
void view_combined(mat4 mat);

void view_projection_matrix(mat4 projMatReturn);

/* translate view by v */
void view_translate(vec2 v);

/* rotate view by angle in deg */
void view_rotate(float angle);

/* zoom view by v */
void view_zoom(vec2 v);

void view_set_position(vec2 pos);

void view_set_rotation(float rot);

/* reset view */
void view_reset(float x, float y, float w, float h);

float view_top(void);
float view_bot(void);
float view_left(void);
float view_right(void);
void  view_rect(OUT SDL_Rect* rect);

void to_view_coords(vec2 src, vec2 dst);
#endif // VIEW_H
