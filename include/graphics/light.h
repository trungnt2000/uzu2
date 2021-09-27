// ngotrung Sat 21 Aug 2021 01:04:59 PM +07
#ifndef GRAPHICS_LIGHT_H
#define GRAPHICS_LIGHT_H
#include "graphics/framebuffer.h"
#include "graphics/types.h"
#include "graphics/view.h"

typedef struct Light
{
  vec2  position;
  vec4  color;
  float intensity;
  float radius;
} Light;

typedef struct LightShader
{
  u32 handle;
  int uLightPositionLoc;
  int uLightColorLoc;
  int uLightInstensityLoc;
  int uLightRadiusLoc;
  int uInvertViewMatrixLoc;
  int uProjectionMatrixLoc;
} LightShader;

typedef struct LightingPass
{
  LightShader shader;
  Framebuffer framebuffer;

  /* for renderer quad */
  u32 vao;
  u32 vbo;
} LightingPass;

int  lighting_pass_init(LightingPass* lightingPass, int scrWidth, int scrHeight);
void lighting_pass_destroy(LightingPass* lightingPass);

void lighting_pass_render(LightingPass*  lightingPass,
                          OthoCamera*          view,
                          const Light*   lights,
                          int            count,
                          const Texture* input);

const Texture* lighting_pass_get_output(LightingPass* lightingPass);

#endif // GRAPHICS_LIGHT_H
