#include "graphics/view.h"
#include "toolbox/common.h"

static vec3  sPos;
static vec3  sScl;
static vec2  sSiz;
static float sRot;
static mat4  sProjMat;
static mat4  sViewMat;
static bool  sDirty;
static mat4  sInvViewMat;
void
view_combined(mat4 viewProjMatReturn)
{

  if (sDirty)
  {
    glm_translate_make(
        sViewMat,
        (vec3){ -(sPos[0] - sSiz[0] / 2.f), -(sPos[1] - sSiz[1] / 2.f), 0 });
    glm_scale(sViewMat, sScl);
    glm_rotate_z(sViewMat, sRot, sViewMat);
    glm_mat4_inv(sViewMat, sInvViewMat);
    sDirty = false;
  }
  glm_mat4_mul(sProjMat, sViewMat, viewProjMatReturn);
  // glm_mat4_copy(sProjMat, outVpMat);
}

void view_projection_matrix(mat4 projMatReturn)
{
  glm_mat4_copy(sProjMat, projMatReturn);
}

void
view_translate(vec2 v)
{
  sDirty = true;
  sPos[0] += v[0];
  sPos[1] += v[1];
}

void
view_rotate(float angle)
{
  sDirty = true;
  sRot += angle;
}

void
view_zoom(vec2 v)
{
  sDirty = true;
  sScl[0] *= v[0];
  sScl[1] *= v[1];
}

void
view_set_position(vec2 pos)
{
  sDirty = true;
  glm_vec3_copy((vec3){ pos[0], pos[1], 0.f }, sPos);
}

void
view_set_rotation(float rot)
{
  sDirty = true;
  sRot = rot;
}
void
view_reset(float x, float y, float w, float h)
{
  sScl[0] = 1.f;
  sScl[1] = 1.f;
  sScl[2] = 1.f;

  sSiz[0] = w;
  sSiz[1] = h;

  sPos[0] = x;
  sPos[1] = y;

  sRot   = 0.f;
  sDirty = true;

  glm_ortho(0, w, h, 0, 100, -100.f, sProjMat);
}

float
view_top(void)
{
  sDirty = true;
  return sPos[1] - sSiz[1] / 2.f;
}

float
view_bot(void)
{
  sDirty = true;
  return sPos[1] + sSiz[1] / 2.f;
}

float
view_left(void)
{
  sDirty = true;
  return sPos[0] - sSiz[0] / 2.f;
}

float
view_right(void)
{
  sDirty = true;
  return sPos[0] + sSiz[0] / 2.f;
}

void
view_rect(SDL_Rect* outRect)
{
  outRect->x = (int)view_left();
  outRect->y = (int)view_top();
  outRect->w = (int)sSiz[0];
  outRect->h = (int)sSiz[1];
}

void to_view_coords(vec2 src, vec2 dst)
{
  vec4 tmp;
  glm_mat4_mulv(sInvViewMat, (vec4){ src[0], src[1], 0.f, 1.f }, tmp);
  dst[0] = tmp[0];
  dst[1] = tmp[1];
}