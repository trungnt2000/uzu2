#include "ecs_priv.h"

#define INDEX_MASK 0xfff

void
_ecs_view_init(ecs_View* view, ecs_Registry* reg, ecs_size_t* types, u32 cnt)
{
  view->exclCnt    = 0;
  view->reqdCnt    = 0;
  ecs_Pool** pools = reg->pools;
  for (u32 i = 0; i < cnt; ++i)
  {
    if (types[i] & ECS_EXCL_MASK)
    {
      ASSERT(view->exclCnt < ECS_VIEW_MAX_COMP_CNT);
      view->exclPools[view->exclCnt++] = pools[types[i] & INDEX_MASK];
    }
    else
    {
      ASSERT(view->reqdCnt < ECS_VIEW_MAX_COMP_CNT);
      view->reqdPools[view->reqdCnt++] = pools[types[i] & INDEX_MASK];
    }
  }

  ASSERT(view->reqdCnt > 0 && "create view need at least 1 component");

  view->fastPath    = 0;
  ecs_Pool* minPool = view->reqdPools[0];
  for (u32 i = 1; i < view->reqdCnt; ++i)
  {
    if (view->reqdPools[i]->count < view->reqdPools[view->fastPath]->count)
    {
      view->fastPath = i;
      minPool        = view->reqdPools[i];
    }
  }

  /* interator begin at the last element in packed array */
  view->dataIter = ecs_pool_data_rbegin(minPool);
  view->dataEnd  = ecs_pool_data_rend(minPool);
  view->ettIter  = ecs_pool_ett_rbegin(minPool);
  view->ettEnd   = ecs_pool_ett_rend(minPool);

  view->tsiz = minPool->traits.size;
}

bool
_ecs_view_next(ecs_View* v, ecs_entity_t* ett, void* comps[])
{
  if (v->ettIter == v->ettEnd)
    return false;

  bool       match = false;
  bool       excl;
  u8*        dataPtr;
  ecs_Pool** exclPools = v->exclPools;
  ecs_Pool** reqdPools = v->reqdPools;

  /* use backward iteration to freely destroy entity */
  while (!match && v->ettIter != v->ettEnd)
  {
    /* save current position of fast path */
    dataPtr = v->dataIter;

    /* move iterator */
    v->dataIter = (u8*)v->dataIter - v->tsiz;
    *ett        = *(v->ettIter--);

    /* check whether or not entity has excluded component */
    excl = false;
    for (u32 i = 0; i < v->exclCnt; ++i)
    {
      if (ecs_pool_contains(exclPools[i], *ett))
      {
        excl = true;
        break;
      }
    }

    if (!excl)
    {
      match = true;
      for (u32 i = 0; i < v->reqdCnt; ++i)
      {
        if (i == v->fastPath)
        {
          comps[i] = dataPtr;
        }
        else
        {
          comps[i] = ecs_pool_get(reqdPools[i], *ett);
          if (comps[i] == NULL)
          {
            match = false;
            break;
          }
        }
      }
    }
  }
  return match;
}
