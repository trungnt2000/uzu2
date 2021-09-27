#include "equipment.h"
#include "components.h"

void
eqm_attach_weapon(ecs_Registry* registry, ecs_entity_t entity, ecs_entity_t weapon)
{
  ecs_add_or_set(registry, entity, HandComp, { .weapon = weapon });
  ecs_add_or_set(registry, weapon, HolderComp, { .holder = entity });
}
