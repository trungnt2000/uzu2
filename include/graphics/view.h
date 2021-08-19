// ngotrung Thu 15 Jul 2021 09:17:19 PM +07
#ifndef VIEW_H
#define VIEW_H
#include "SDL.h"
#include "cglm/cglm.h"
#include "toolbox.h"

typedef struct View
{
   vec3  position;
   vec3  scale;
   vec2  size;
   float rotation;
   mat4  projMat;
   mat4  viewMat;
   bool  dirty;
   mat4  invViewMat;
}View;

/* get combined matrix */
void view_combined(View* view, mat4 mat);

void view_projection_matrix(View* view, mat4 projMatReturn);

/* translate view by v */
void view_translate(vec2 v);

/* rotate view by angle in deg */
void view_rotate(View* view, float angle);

/* zoom view by v */
void view_zoom(View* view, vec2 v);

void view_set_position(View* view, vec2 pos);

void view_set_rotation(View* view, float rot);

/* reset view */
void view_reset(View* view, float x, float y, float w, float h);

float view_top(View* view);
float view_bot(View* view);
float view_left(View* view);
float view_right(View* view);

void to_view_coords(View* view, vec2 src, vec2 dst);
#endif // VIEW_H
