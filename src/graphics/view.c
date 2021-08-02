#include "graphics/view.h"
#include "toolbox/common.h"

static vec3  sPos;
static vec3  sScl;
static vec2  sSiz;
static float sRot;
static mat4  sProjMat;
static mat4  sViewMat; /* combined view and projection matrix */
static BOOL  sDirty;

void
view_combined(mat4 outVpMat)
{

  if (sDirty)
  {
#if 0
        float angle  = glm_rad(sRot);
        float cosine = SDL_cosf(angle);
        float sine   = SDL_sinf(angle);
        float tx      = -sPos[0] * cosine - sPos[1] * sine + sPos[0];
        float ty      = sPos[0] * sine - sPos[1] * cosine + sPos[1];

        // Projection components
        float a =  2.f / sSiz[0];
        float b = -2.f / sSiz[1];
        float c = -a * sPos[0];
        float d = -b * sPos[1];

        // Rebuild the projection matrix
        ( a * cosine, a * sine,   a * tx + c,
         -b * sine,   b * cosine, b * ty + d,
          0.f,        0.f,        1.f        );
#endif
    glm_translate_make(sViewMat, (vec2){ -(sPos[0] - sSiz[0] / 2.f), - (sPos[1] - sSiz[1] / 2.f) });
    sDirty = UZU_FALSE;
  }
  glm_mat4_mul(sProjMat, sViewMat, outVpMat);
  //glm_mat4_copy(sProjMat, outVpMat);
}

void
view_translate(vec2 v)
{
  sDirty = UZU_TRUE;
  sPos[0] += v[0];
  sPos[1] += v[1];
}

void
view_rotate(float angle)
{
  sDirty = UZU_TRUE;
  sRot += angle;
}

void
view_zoom(vec2 v)
{
  sDirty = UZU_TRUE;
  sScl[0] *= v[0];
  sScl[1] *= v[1];
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
  sDirty = UZU_TRUE;

  glm_ortho(0, w, h, 0, 100, -100.f, sProjMat);
}

float
view_top(void)
{
  return sPos[1] - sSiz[1] / 2.f;
}

float
view_bot(void)
{
  return sPos[1] + sSiz[1] / 2.f;
}

float
view_left(void)
{
  return sPos[0] - sSiz[0] / 2.f;
}

float
view_right(void)
{
  return sPos[0] + sSiz[0] / 2.f;
}

void
viww_rect(SDL_Rect* outRect)
{
  outRect->x = (int) view_left();
  outRect->y = (int) view_top();
  outRect->w = (int) sSiz[0];
  outRect->h = (int) sSiz[1];
}
