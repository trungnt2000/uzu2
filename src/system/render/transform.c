#include "components.h"
#include "ecs.h"

typedef struct Dependencies
{
  const _TransformChanged* dummy;
  _Transform*              transform;
  _TransformMatrix*        transformMatrix;
} Dependencies;

void
system_rendering_transform_update(ecs_Registry* registry)
{
  ecs_View     view;
  ecs_entity_t ett;
  Dependencies deps;
  _Transform*  transform;

  ecs_view_init(&view,
                registry,
                { TransformChanged, Transform, TransformMatrix });
  while (ecs_view_next(&view, &ett, &deps))
  {
    transform = deps.transform;

#if 0
    glm_scale2d_make(mat, transform->scale);
    glm_rotate2d(mat, glm_rad(transform->rotation));
    glm_translate2d(mat, transform->position);
#endif
#if 1
    float angle  = -glm_rad(transform->rotation);
    float cosine = SDL_cosf(angle);
    float sine   = SDL_sinf(angle);
    float sxc    = transform->scale[0] * cosine;
    float syc    = transform->scale[1] * cosine;
    float sxs    = transform->scale[0] * sine;
    float sys    = transform->scale[1] * sine;
    float tx     = transform->position[0];
    float ty     = transform->position[1];

    mat3 transformMatrix2 = {
      {sxc, -sxs, 0.f },
      {sys,  syc, 0.f },
      { tx,   ty, 1.f },
    };
    
    glm_mat3_copy(transformMatrix2, deps.transformMatrix->value);
#endif
    ecs_rmv(registry, ett, TransformChanged);
  }
}
