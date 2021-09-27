// ngotrung Thu 15 Jul 2021 09:17:19 PM +07
#ifndef VIEW_H
#define VIEW_H
#include "SDL.h"
#include "cglm/cglm.h"
#include "toolbox.h"

typedef struct OthoCamera
{
    mat4 inv_view_matrix;
    mat4 projection_matrix;
    mat4 view_matrix;

    vec3  position;
    vec3  scale;
    vec2  size;
    float rotation;
    bool  dirty;
} OthoCamera;

void otho_camera_get_view_projection_matrix(OthoCamera* cam, mat4 viewProjMatReturn);

void otho_camera_get_projection_matrix(OthoCamera* cam, mat4 projMatReturn);

void otho_camera_get_invert_view_matrix(OthoCamera* cam, mat4 invertViewMatReturn);

void otho_camera_get_view_matrix(OthoCamera* cam, mat4 viewMatReturn);

void otho_camera_to_view_coords(OthoCamera* cam, vec2 src, vec2 dst);

/* translate cam by v */
void otho_camera_translate(OthoCamera* cam, vec2 v);

/* rotate cam by angle in deg */
void otho_camera_rotate(OthoCamera* cam, float angle);

/* zoom cam by v */
void otho_camera_zoom(OthoCamera* cam, vec2 v);

void otho_camera_set_position(OthoCamera* cam, vec2 pos);

void otho_camera_set_rotation(OthoCamera* cam, float rot);

void otho_camera_set_zoom(OthoCamera* cam, vec2 z);

/* reset cam */
void otho_camera_reset(OthoCamera* cam, float x, float y, float w, float h);

float otho_camera_view_top(OthoCamera* cam);
float otho_camera_view_bot(OthoCamera* cam);
float otho_camera_view_left(OthoCamera* cam);
float otho_camera_view_right(OthoCamera* cam);

#endif // VIEW_H
