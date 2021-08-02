#include "ecs_priv.h"

static bool
contains_intl(ecs_Group* group, ecs_entity_t ett)
{
  for (int i = 0; i < group->exclCnt; ++i)
  {
    if (ecs_pool_contains(group->exclPools[i], ett))
    {
      return false;
    }
  }

  for (int i = 0; i < group->ownCnt; ++i)
  {
    if (!ecs_pool_contains(group->ownPools[i], ett))
    {
      return false;
    }
  }
  return true;
}

bool
ecs_group_contains(ecs_Group* g, ecs_entity_t ett)
{
  // this group contain given entity when first pool contains the entity
  // and it index must smaller than group size
  ecs_size_t idx = ecs_pool_index(g->ownPools[0], ett);
  return idx != ECS_NULL_IDX && idx < g->size;
}

static ecs_size_t
add_hook_impl(ecs_Group*   group,
              ecs_Pool*    pool,
              ecs_entity_t ett,
              ecs_size_t   idx)
{
  if (contains_intl(group, ett))
  {
    // swap given entity to right after last entity in group
    ecs_size_t newIdx = group->size;
    ecs_Pool** pools  = group->ownPools;
    for (ecs_size_t i = 0; i < group->ownCnt; ++i)
    {
      ecs_entity_t lhs = ett;
      ecs_entity_t rhs = pools[i]->entities[newIdx];
      if (lhs != rhs)
      {
        ecs_pool_swp(pools[i], lhs, rhs);
      }
    }
    idx = newIdx;

    group->size++;
    if (group->child != NULL)
    {
      idx = add_hook_impl(group->child, pool, ett, idx);
    }
  }
  return idx;
}

static ecs_size_t
rmv_hook_impl(ecs_Group*   group,
              ecs_Pool*    pool,
              ecs_entity_t ett,
              ecs_size_t   idx)
{
  if (ecs_group_contains(group, ett))
  {
    if (group->child != NULL)
    {
      idx = rmv_hook_impl(group, pool, ett, idx);
    }
    // move given entity to the end of this group
    // all entities are inside same group will have same index
    ecs_size_t   lastIdx = group->size - 1;
    ecs_entity_t lhs     = ett;
    ecs_entity_t rhs     = pool->entities[lastIdx];
    ecs_Pool**   pools   = group->ownPools;
    if (lhs != rhs)
    {
      for (ecs_size_t i = 0; i < group->ownCnt; ++i)
      {
        ecs_pool_swp(pools[i], lhs, rhs);
      }
    }
    group->size--;
    idx = lastIdx;
  }
  return idx;
}

static bool
has_pool(ecs_Pool** pools, ecs_size_t cnt, ecs_Pool* p)
{
  for (int i = 0; i < cnt; ++i)
    if (pools[i] == p)
      return true;

  return false;
}

/* Check whether or not self group is child of other group */
static bool
is_child_of(ecs_Group* self, ecs_Group* other)
{
  // this group must have more types to be child of other group
  if (self->ownCnt < other->ownCnt)
    return false;

  // and all types that parent group has this group must have too
  for (int i = 0; i < other->ownCnt; ++i)
    if (!has_pool(self->ownPools, self->ownCnt, other->ownPools[i]))
      return false;

  // and it must not contain any excluded types of other group
  for (int i = 0; i < other->exclCnt; ++i)
    if (has_pool(self->ownPools, self->ownCnt, other->exclPools[i]))
      return false;

  return true;
}

/* Check whether or not self group is parent of other group */
static bool
is_parent_of(ecs_Group* self, ecs_Group* other)
{

  // to be parent of other group this group must have fewer types than
  // other group
  if (self->ownCnt > other->ownCnt)
    return false;

  // and all types it owns that group must have too
  for (int i = 0; i < self->ownCnt; ++i)
    if (!has_pool(other->ownPools, other->ownCnt, self->ownPools[i]))
      return false;

  // and that group must not contain any excluded types of this group
  for (int i = 0; i < self->exclCnt; ++i)
    if (has_pool(other->ownPools, other->ownCnt, self->exclPools[i]))
      return false;

  return true;
}

static void
ecs_group_refresh(ecs_Group* g, ecs_size_t first, ecs_size_t last)
{
}

static void _esc_pool_init_free_variables(ecs_size_t Tc, ecs_Pool** own, ecs_Pool** excl, ecs_Pool** shar) {
  SDL_free(own);
  SDL_free(excl);
  SDL_free(shar);
}

void
_ecs_group_init(ecs_Group*    group,
                ecs_Registry* registry,
                ecs_size_t*   Ts,
                ecs_size_t    Tc)
{
  
  ecs_Pool**  own;
  ecs_Pool**  excl;
  ecs_Pool**  shar;
  
  own = SDL_malloc(sizeof(ecs_Pool*) * Tc);
  excl = SDL_malloc(sizeof(ecs_Pool*) * Tc);
  shar = SDL_malloc(sizeof(ecs_Pool*) * Tc);

  ecs_Pool** pools = registry->pools;

  group->exclCnt = 0;
  group->ownCnt  = 0;
  group->sharCnt = 0;
  group->size    = 0;
  group->child   = NULL;

  for (int i = 0; i < Tc; ++i)
  {
    if (Ts[i] & ECS_EXCL_MASK)
    {
      excl[group->exclCnt++] = pools[Ts[i] & ECS_TYPE_MASK];
    }
    else if (Ts[i] & ECS_SHARED_MASK)
    {
      shar[group->sharCnt++] = pools[Ts[i] & ECS_TYPE_MASK];
    }
    else
    {
      own[group->ownCnt++] = pools[Ts[i] & ECS_TYPE_MASK];
    }
  }

  group->exclPools = SDL_malloc(sizeof(ecs_Pool*) * group->exclCnt);
  group->ownPools  = SDL_malloc(sizeof(ecs_Pool*) * group->ownCnt);
  group->sharPools = SDL_malloc(sizeof(ecs_Pool*) * group->sharCnt);

  SDL_memcpy(group->exclPools, excl, group->exclCnt * sizeof(ecs_Pool*));
  SDL_memcpy(group->ownPools, own, group->ownCnt * sizeof(ecs_Pool*));
  SDL_memcpy(group->sharPools, shar, group->sharCnt * sizeof(ecs_Pool*));

  for (int i = 0; i < registry->groupCnt; ++i)
  {
    ecs_Group* otherGroup = registry->groups[i];
    if (is_child_of(group, otherGroup))
    {
      // walk down to find best place where no more group in this chain or
      // given this group is parent of next group
      while (otherGroup->child != NULL && is_child_of(group, otherGroup->child))
        otherGroup = otherGroup->child;

      // double check in case this group is not parent of next in case the
      // next group has excluded types
      if (otherGroup->child == NULL || is_parent_of(group, otherGroup->child))
      {
        group->child      = otherGroup->child;
        otherGroup->child = group;

        group->size = group->child != NULL ? group->child->size : 0;
        ecs_group_refresh(group, group->size, otherGroup->size - 1);
        _esc_pool_init_free_variables(Tc, own, excl, shar);
        return;
      }
    }
    else if (is_parent_of(group, otherGroup))
    {
      // make it become toplevel group
      registry->groups[i] = group;

      // initialize
      group->child = otherGroup;

      group->size  = otherGroup->size;
      _esc_pool_init_free_variables(Tc, own, excl, shar);
      return;
    }
  }

  bool sortable = true;
  for (int i = 0; i < group->ownCnt; ++i)
  {
    if (!ecs_pool_sortable(group->ownPools[i]))
    {
      sortable = false;
      break;
    }
  }

  if (sortable)
  {
    registry->groupCnt += 1;
    registry->groups =
        SDL_realloc(registry->groups, sizeof(ecs_Group*) * registry->groupCnt);
    registry->groups[registry->groupCnt - 1] = group;

    for (int i = 0; i < group->ownCnt; ++i)
    {
      ecs_pool_take_ownership(group->ownPools[i],
                              (ecs_AddHook)add_hook_impl,
                              (ecs_RmvHook)rmv_hook_impl,
                              group);
    }

    _esc_pool_init_free_variables(Tc, own, excl, shar);
    return;
  }

  _esc_pool_init_free_variables(Tc, own, excl, shar);
  ASSERT(0 && "Could not create group");
}

void
ecs_group_destroy(ecs_Group* group)
{
  SDL_free(group->exclPools);
  SDL_free(group->ownPools);
  SDL_free(group->sharPools);
}
