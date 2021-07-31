#include "ecs_priv.h"

static bool
ecs_group_contains(ecs_Group* group, ecs_entity_t ett)
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

static ecs_size_t
add_hook_impl(ecs_Group*   group,
              ecs_Pool*    pool,
              ecs_entity_t ett,
              ecs_size_t   idx)
{
  if (ecs_group_contains(group, ett))
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
        ecs_pool_swap(pools[i], lhs, rhs);
      }
    }

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
    // TODO: remove given entity to the end of this group
    /* all entities are inside same group will have same index */
    ecs_size_t   lastIdx = group->size - 1;
    ecs_entity_t lhs     = ett;
    ecs_entity_t rhs     = pool->entities[lastIdx];
    ecs_Pool**   pools   = group->ownPools;
    if (lhs != rhs)
    {
      for (ecs_size_t i = 0; i < group->ownCnt; ++i)
      {
        ecs_pool_swap(pools[i], lhs, rhs);
      }
    }
    group->size--;
  }
  return idx;
}

bool
is_sub_set_of(ecs_Pool** me, ecs_size_t meCnt, ecs_Pool** them, ecs_size_t);

bool
is_child_of(ecs_Group* a, ecs_Group* b)
{
  if (a->ownCnt < b->ownCnt)
    return false;
}

bool
is_parent_of(ecs_Group* a, ecs_Group* b)
{
  if (a->ownCnt > b->ownCnt)
    return false;
}

void
ecs_group_refresh(ecs_Group* g, ecs_size_t first, ecs_size_t last)
{
}

void
_ecs_group_init(ecs_Group*    group,
                ecs_Registry* registry,
                ecs_size_t*   Ts,
                ecs_size_t    Tc)
{

  ecs_size_t own[Tc];
  ecs_size_t excl[Tc];
  ecs_size_t shar[Tc];

  group->exclCnt = 0;
  group->ownCnt  = 0;
  group->sharCnt = 0;
  group->size    = 0;
  group->child   = NULL;

  for (int i = 0; i < Tc; ++i)
  {
    if (Ts[i] & ECS_EXCL_MASK)
    {
      excl[group->exclCnt++] = Ts[i] & ECS_TYPE_MASK;
    }
    else if (Ts[i] & ECS_SHARED_MASK)
    {
      shar[group->sharCnt++] = Ts[i] & ECS_TYPE_MASK;
    }
    else
    {
      own[group->ownCnt++] = Ts[i] & ECS_TYPE_MASK;
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
    ecs_Group* b = registry->groups[i];
    if (is_child_of(group, b))
    {
      // walk down to find best place where no more group in this chain or
      // given group is parent of next group
      while (b->child != NULL && is_child_of(group, b->child))
        b = b->child;

      group->child = b->child;
      b->child     = group;

      group->size = group->child != NULL ? group->child->size : 0;
      ecs_group_refresh(group, group->size, b->size - 1);

      return;
    }
    else if (is_parent_of(group, b))
    {
      // make it become toplevel group
      registry->groups[i] = group;

      // initialize
      group->child = b;
      group->size  = 0;
      return;
    }
  }

  bool sortable = true;
  for (int i = 0; i < Tc; ++i)
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
  }
}
