#include "ecs.h"
#include "ecs_priv.h"

#define ECS_DEFAULT_SIZE 16

#define ASSERT_VALID_ENTITY(registry, entity)                                  \
  ASSERT(ecs_is_valid(registry, entity) && "invalid entity")
#define ASSERT_VALID_TYPE_ID(registry, typeId)                                 \
  ASSERT((typeId) < registry->typeCnt && "invalid typeId")

static void
clear_callback(void* udata, ecs_entity_t ett, SDL_UNUSED void* unused)
{
  ecs_destroy((ecs_Registry*)udata, ett);
}

ecs_Registry*
ecs_registry_create(const ecs_TypeTraits* typeTraitsArr, ecs_size_t cnt)
{
  ecs_Registry* reg = SDL_malloc(sizeof(ecs_Registry));
  reg->typeCnt      = cnt;
  reg->typeTraits   = typeTraitsArr;
  reg->pools        = SDL_calloc(cnt, sizeof(void*));
  reg->groupCnt     = 0;
  reg->groups       = NULL;
  for (int i = 0; i < cnt; ++i)
  {
    reg->pools[i] = ecs_pool_create(typeTraitsArr[i], ECS_DEFAULT_SIZE);
  }
  reg->count      = 0;
  reg->size       = ECS_DEFAULT_SIZE;
  reg->destroyIdx = 0;
  reg->entities   = SDL_malloc(ECS_DEFAULT_SIZE * sizeof(ecs_entity_t));

  for (int i = 0; i < ECS_DEFAULT_SIZE - 1; ++i)
    reg->entities[i] = ECS_ENT(i + 1, 0);

  reg->entities[ECS_DEFAULT_SIZE - 1] = ECS_ENT(ECS_NULL_IDX, 0);
  return reg;
}

void
ecs_registry_free(ecs_Registry* reg)
{
  if (reg != NULL)
  {
    ecs_each(reg, clear_callback, reg);
    for (int i = 0; i < reg->typeCnt; ++i)
      ecs_pool_free(reg->pools[i]);
    SDL_free(reg->pools);
    SDL_free(reg->entities);
    SDL_free(reg);
  }
}

ecs_entity_t
ecs_create(ecs_Registry* reg)
{
  ecs_size_t ver;
  ecs_size_t idx;

  if (reg->destroyIdx == ECS_NULL_IDX)
  {
    reg->size *= 2;
    reg->entities =
        SDL_realloc(reg->entities, reg->size * sizeof(ecs_entity_t));
    for (int i = reg->count; i < reg->size - 1; ++i)
    {
      reg->entities[i] = ECS_ENT(i + 1, 0);
    }
    reg->entities[reg->size - 1] = ECS_ENT(ECS_NULL_IDX, 0);
    reg->destroyIdx              = reg->count;
  }

  idx                = reg->destroyIdx;
  ver                = ECS_ENT_VER(reg->entities[idx]);
  reg->destroyIdx    = ECS_ENT_IDX(reg->entities[idx]);
  reg->entities[idx] = ECS_ENT(idx, ver);
  reg->count++;

  return ECS_ENT(idx, ver);
}

INLINE void
recycle(ecs_Registry* reg, ecs_entity_t ett)
{
  ecs_size_t ent_ver;
  ecs_size_t ent_idx;

  ent_ver = (ett >> ECS_ENT_VER_SHIFT) & 0xffff;
  ent_idx = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;

  reg->entities[ent_idx] = ECS_ENT(reg->destroyIdx, ent_ver + 1);
  reg->destroyIdx        = ent_idx;
  reg->count--;
}

void
ecs_destroy(ecs_Registry* reg, ecs_entity_t ett)
{
  ecs_rmv_all(reg, ett);
  recycle(reg, ett);
}

void*
_ecs_add(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ASSERT_VALID_ENTITY(reg, ett);
  return ecs_pool_add(reg->pools[typeId], ett);
}

void
ecs_rmv(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ASSERT_VALID_ENTITY(reg, ett);
  ecs_pool_rmv(reg->pools[typeId], ett);
}

void
ecs_rmv_all(ecs_Registry* reg, ecs_entity_t ett)
{
  ASSERT_VALID_ENTITY(reg, ett);
  ecs_Pool** pools;

  pools = reg->pools;
  for (int i = 0; i < reg->typeCnt; ++i)
  {
    ecs_pool_rmv(pools[i], ett);
  }
}

void*
_ecs_get(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ASSERT_VALID_ENTITY(reg, ett);
  return ecs_pool_get(reg->pools[typeId], ett);
}

void
ecs_each(ecs_Registry* reg, ecs_Callback callback, void* userData)
{
  ecs_size_t    size;
  ecs_entity_t* entities;

  size     = reg->size;
  entities = reg->entities;
  if (reg->destroyIdx == ECS_NULL_IDX)
    for (int i = size - 1; i >= 0; --i)
      callback(userData, entities[i], NULL);
  else
    for (int i = size - 1; i >= 0; --i)
      if (ECS_ENT_IDX(entities[i]) == i)
        callback(userData, entities[i], NULL);
}

void
_ecs_raw(ecs_Registry*        reg,
         ecs_size_t           typeId,
         const ecs_entity_t** etts,
         void*                comps[],
         ecs_size_t*          cnt)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ecs_pool_data(reg->pools[typeId], comps, etts, cnt);
}

bool
ecs_has(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ASSERT_VALID_ENTITY(reg, ett);
  return ecs_pool_contains(reg->pools[typeId], ett);
}

void
ecs_clear(ecs_Registry* reg)
{
  ecs_each(reg, clear_callback, reg);
}

void*
_ecs_set(ecs_Registry* reg,
         ecs_entity_t  ett,
         ecs_size_t    typeId,
         const void*   data)
{
  ASSERT_VALID_TYPE_ID(reg, typeId);
  ASSERT_VALID_ENTITY(reg, ett);

  return ecs_pool_set(reg->pools[typeId], ett, data);
}

void
_ecs_getn(ecs_Registry*     reg,
          ecs_entity_t      ett,
          const ecs_size_t* types,
          ecs_size_t        cnt,
          void*             arr[])
{
  ASSERT_VALID_ENTITY(reg, ett);

  u32        typeCnt = reg->typeCnt;
  ecs_Pool** pools   = reg->pools;
  for (int i = 0; i < cnt; ++i)
  {
    ASSERT(types[i] < typeCnt);
    if ((arr[i] = ecs_pool_get(pools[types[i]], ett)) == NULL)
      break;
  }
}

ecs_entity_t
ecs_cpy(ecs_Registry* dstReg, ecs_Registry* srcReg, ecs_entity_t srcEtt)
{
  ecs_entity_t cpyEtt;
  void*        srcMem;
  ASSERT_VALID_ENTITY(srcReg, srcEtt);
  ASSERT(dstReg->typeTraits == srcReg->typeTraits);

  cpyEtt = ecs_create(dstReg);

  for (u32 typeId = 0; typeId < dstReg->typeCnt; ++typeId)
    if ((srcMem = ecs_pool_get(srcReg->pools[typeId], srcEtt)) != NULL)
      ecs_pool_cpy(dstReg->pools[typeId], cpyEtt, srcMem);

  return cpyEtt;
}

bool
ecs_is_valid(ecs_Registry* reg, ecs_entity_t ett)
{
  ecs_size_t idx = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  return (idx < reg->size) && (reg->entities[idx] == ett);
}

void
ecs_connect(ecs_Registry* reg,
            int           event,
            ecs_size_t    typeId,
            ecs_Callback  callback,
            void*         ctx)
{
  ASSERT(typeId < reg->typeCnt && "Invalid type");
  ecs_pool_connect(reg->pools[typeId], event, callback, ctx);
}

void
ecs_disconnect(ecs_Registry* reg,
               int           event,
               ecs_size_t    typeId,
               ecs_Callback  callback,
               void*         ctx)
{
  ASSERT(typeId < reg->typeCnt && "Invalid type");
  ecs_pool_disconnect(reg->pools[event], event, callback, ctx);
}
