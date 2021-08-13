// ngotrung Sat 24 Jul 2021 11:58:27 PM +07
#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "cglm/cglm.h"
#include "ecs.h"
#include "font_loader.h"
#include "graphics.h"
#include "text_format.h"
#include "toolbox/common.h"

enum
{
  Transform,
  Sprite,
  TransformChanged,
  TransformMatrix,
  AnimationPool,
  DrawOrder,
  Text,
  COMPONENT_CNT
};

extern const ecs_TypeTraits gCompTraits[];

typedef struct _Text
{
  u32*             codePoints;
  unsigned int     codePointsLength;
  vec3             origin;
  vec2             size;
  const FontAtlas* atlas;
} _Text;

typedef struct _Paragraph
{
  IntRect         region;
  ParagraphFormat format;
} _Paragraph;

typedef struct _Transform
{
  vec3  position;
  vec2  scale;
  float rotation;
} _Transform;

/* this tag component use in case some thing have changed
 * our transform component */
typedef struct _TransformChanged
{
  int dummy;
} _TransformChanged;

typedef struct _Sprite
{
  vec2          origin;
  vec2          size;
  vec4          color;
  TextureRegion textureRegion;
} _Sprite;

typedef struct _TransformMatrix
{
  mat3 value;
} _TransformMatrix;

typedef struct _AnimationPool
{
  Animation* anims;
  int        animCnt;
  float      elapsedTime;
} _AnimationPool;

typedef struct _DrawOrder
{
  float value;
} _DrawOrder;
#endif // COMPONENTS_H
