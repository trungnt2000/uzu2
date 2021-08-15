#include "components.h"
#include "ecs.h"

typedef union Dependencies
{
  void* components[2];
  struct
  {
    _Transform*      transform;
    const _Velocity* velocity;
  };
} Dependencies;

void
system_motion_update(ecs_Registry* registry, float deltaTime)
{
  ecs_View     view;
  ecs_entity_t ett;
  Dependencies deps;
  float*       pos;
  const float* vel;
  // same order in Dependencies struct
  ecs_view_init(&view, registry, { Transform, Velocity });

  while (ecs_view_next(&view, &ett, deps.components))
  {
    pos = deps.transform->position;
    vel = deps.velocity->value;

    pos[0] += vel[0] * deltaTime;
    pos[1] += vel[1] * deltaTime;

    // tag this entity that it have changed position
    ecs_add_or_set(registry, ett, TransformChanged, { 0 });
  }
}
