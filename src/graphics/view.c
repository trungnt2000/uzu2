#include "graphics/view.h"
#include "toolbox/common.h"

void
view_combined(View* view, mat4 viewProjMatReturn)
{

  if (view->dirty)
  {
    glm_translate_make(view->viewMat,
        (vec3){ -(view->position[0] - view->size[0] / 2.f), -(view->position[1] - view->size[1] / 2.f), 0 });
    glm_scale(view->viewMat, view->scale);
    glm_rotate_z(view->viewMat, view->rotation, view->viewMat);
    glm_mat4_inv(view->viewMat, view->invViewMat);
    view->dirty = false;
  }
  glm_mat4_mul(view->projMat, view->viewMat, viewProjMatReturn);
  // glm_mat4_copy(sProjMat, outVpMat);
}

void
view_projection_matrix(View* view, mat4 projMatReturn)
{
  glm_mat4_copy(view->projMat, projMatReturn);
}

void
view_translate(View* view, vec2 v)
{
  view->dirty = true;
  view->position[0] += v[0];
  view->position[1] += v[1];
}

void
view_rotate(View* view, float angle)
{
  view->dirty = true;
  view->rotation += angle;
}

void
view_zoom(View* view, vec2 v)
{
  view->dirty = true;
  view->scale[0] *= v[0];
  view->scale[1] *= v[1];
}

void
view_set_position(View* view, vec2 pos)
{
  view->dirty = true;
  glm_vec3_copy((vec3){ pos[0], pos[1], 0.f }, view->position);
}

void
view_set_rotation(View* view, float rot)
{
  view->dirty = true;
  view->rotation   = rot;
}
void
view_reset(View* view, float x, float y, float w, float h)
{
  view->scale[0] = 1.f;
  view->scale[1] = 1.f;
  view->scale[2] = 1.f;

  view->size[0] = w;
  view->size[1] = h;

  view->position[0] = x;
  view->position[1] = y;

  view->rotation = 0.f;
  view->dirty = true;

  glm_ortho(0, w, h, 0, 100, -100.f, view->projMat);
}

float
view_top(View* view)
{
  view->dirty = true;
  return view->position[1] - view->size[1] / 2.f;
}

float
view_bot(View* view)
{
  view->dirty = true;
  return view->position[1] + view->size[1] / 2.f;
}

float
view_left(View* view)
{
  view->dirty = true;
  return view->position[0] - view->size[0] / 2.f;
}

float
view_right(View* view)
{
  view->dirty = true;
  return view->position[0] + view->size[0] / 2.f;
}

void
to_view_coords(View* view, vec2 src, vec2 dst)
{
  vec4 tmp;
  glm_mat4_mulv(view->invViewMat, (vec4){ src[0], src[1], 0.f, 1.f }, tmp);
  dst[0] = tmp[0];
  dst[1] = tmp[1];
}