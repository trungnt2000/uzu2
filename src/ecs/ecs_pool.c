#include "ecs_priv.h"
#include "toolbox/memory.h"

/**
 * i1 means sparse index 1
 * j1 means dense index 1
 */

#define PAGE_CNT ECS_SPARSE_SET_PAGE_CNT

#define PAGE_SIZ ECS_SPARSE_SET_PAGE_SIZ

#define TOMBSTONE (0xffff)

#define PAGE(idx) ((idx) / PAGE_SIZ)
#define OFFSET(idx) ((idx) & (PAGE_SIZ - 1u))

#define ALIGNMENT_EXTRA_SPACE 64u

static inline void
dataswp(u8* data, register size_t ts, u32 j1, u32 j2)
{
  register u8* mem1 = data + ts * j1;
  register u8* mem2 = data + ts * j2;

  do
  {
    u8 tmp = *mem1;
    *mem1++ = *mem2;
    *mem2++ = tmp;
  } while (--ts > 0);
}

static inline void
datacpy(u8* data, size_t ts, u32 dstIdx, u32 srcIdx)
{
  u8* srcMem = data + ts * srcIdx;
  u8* dstMem = data + ts * dstIdx;
  SDL_memcpy(dstMem, srcMem, ts);
}

static inline ecs_size_t*
assure_page(ecs_size_t** sparse, u32 page)
{
  ASSERT(page < PAGE_CNT);
  if (sparse[page] == NULL)
  {
    sparse[page] = SDL_malloc(sizeof(ecs_size_t) * PAGE_SIZ);
    ecs_size_t* mem = sparse[page];
    for (int i = 0; i < PAGE_SIZ; ++i)
      mem[i] = TOMBSTONE;
  }
  return sparse[page];
}

static inline void*
memoffset(ecs_Pool* p, int idx)
{
  return (char*)p->data + p->traits.size * idx;
}

static void* assure_alignment(void* buffer, size_t alignment, size_t data_size) {
  size_t buffer_size = data_size + ALIGNMENT_EXTRA_SPACE;
  if (align(alignment, data_size, &buffer, &buffer_size) == NULL) {
    ASSERT_MSG(false, "extra space is too small for data to be aligned");
  }
  return buffer;
}


static inline void
grow_if_need(ecs_Pool* p)
{
  if (p->count == p->size - 1)
  {
    p->size = p->size * 2;
    p->entities = SDL_realloc(p->entities, p->size * sizeof(ecs_entity_t));
    const size_t data_size = p->size * p->traits.size;
    p->data_buffer = SDL_realloc(p->data_buffer, data_size + ALIGNMENT_EXTRA_SPACE);
    p->data = assure_alignment(p->data_buffer, p->traits.align, data_size);
  }
}

ecs_Pool*
ecs_pool_create(ecs_TypeTraits traits, ecs_size_t size)
{
  ecs_Pool* p = SDL_malloc(sizeof(ecs_Pool));

  /* initialize */
  p->entities = SDL_malloc(sizeof(ecs_entity_t) * size);

  const size_t data_size = traits.size * size;
  p->data_buffer  = SDL_malloc(data_size + ALIGNMENT_EXTRA_SPACE);
  ASSERT_MSG(p->data_buffer, "unable to allocate ecs_pool buffer");

  p->data = assure_alignment(p->data_buffer, traits.align, size);

  p->size        = size;
  p->count       = 0;
  p->traits      = traits;
  p->addHook     = NULL;
  p->rmvHook     = NULL;
  p->hookCtx     = NULL;

  for (int i = 0; i < ECS_SIG_CNT; ++i)
    ecs_signal_init(&p->signal[i]);

  SDL_memset(p->sparse, 0, sizeof(ecs_size_t*) * PAGE_CNT);
  for (u32 i = 0; i < p->size; ++i)
    p->entities[i] = ECS_NULL_ENT;
  return p;
}

void
ecs_pool_free(ecs_Pool* p)
{
  /* free page memory */
  for (u32 i = 0; i < PAGE_CNT; ++i)
    if (p->sparse[i] != NULL)
      SDL_free(p->sparse[i]);

  /* call destructor on all component */
  if (p->traits.fini != NULL)
  {
    u8* mem = p->data;
    for (u32 i = 0; i < p->count; ++i)
    {
      p->traits.fini(mem);
      mem = mem + p->traits.size;
    }
  }
  SDL_free(p->data_buffer);
  SDL_free(p->entities);
  SDL_free(p);
}

void*
ecs_pool_add(ecs_Pool* p, ecs_entity_t ett)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = PAGE(idx);
  u32 offset = OFFSET(idx);

  ecs_size_t* pagemem = assure_page(p->sparse, page);

  ASSERT(pagemem[offset] == TOMBSTONE);

  grow_if_need(p);

  ecs_size_t j    = p->count;
  pagemem[offset] = j;
  p->entities[j]  = ett;
  void* mem       = memoffset(p, j);
  if (p->traits.init != NULL)
    p->traits.init(mem);
  ecs_signal_emit(&p->signal[ECS_SIG_ADD], ett, mem);
  p->count++;

  if (p->addHook != NULL)
    p->addHook(p->hookCtx, p, ett, j);
  return mem;
}

void*
ecs_pool_get(ecs_Pool* p, ecs_entity_t ett)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = PAGE(idx);
  u32 offset = OFFSET(idx);
  if (p->sparse[page] == NULL)
    return NULL;
  ecs_size_t j = p->sparse[page][offset];
  return j != TOMBSTONE ? memoffset(p, j) : NULL;
}

void*
ecs_pool_cpy(ecs_Pool* p, ecs_entity_t ett, const void* other)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = PAGE(idx);
  u32 offset = OFFSET(idx);

  ecs_size_t* pagemem = assure_page(p->sparse, page);
  ASSERT(pagemem[offset] == TOMBSTONE);

  grow_if_need(p);

  ecs_size_t j    = p->count;
  pagemem[offset] = j;
  p->entities[j]  = ett;
  void* mem       = memoffset(p, j);
  if (p->traits.cpy != NULL)
    p->traits.cpy(mem, other);
  else
    SDL_memcpy(mem, other, p->traits.size);

  ecs_signal_emit(&p->signal[ECS_SIG_ADD], ett, mem);
  p->count++;
  // TODO: do some stuffs like sorting or grouping
  return mem;
}

bool
ecs_pool_rmv(ecs_Pool* p, ecs_entity_t ett)
{
  u32        idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32        page   = PAGE(idx);
  u32        offset = OFFSET(idx);
  ecs_size_t i2, j1, j2;
  if (p->sparse[page] == NULL)
    return UZU_FALSE;

  j1 = p->sparse[page][offset];
  if (j1 == TOMBSTONE)
    return UZU_FALSE;

  j2 = p->count - 1;
  i2 = (p->entities[j2] >> ECS_ENT_IDX_SHIFT) & 0xffff;

  void* mem = memoffset(p, j1);
  ecs_signal_emit(&p->signal[ECS_SIG_RMV], ett, mem);

  // call destructor if any
  if (p->traits.fini != NULL)
    p->traits.fini(mem);

  // invoke hook function
  if (p->rmvHook != NULL)
    j1 = p->rmvHook(p->hookCtx, p, j1);

  p->entities[j1] = p->entities[j2];
  datacpy(p->data, p->traits.size, j1, j2);
  p->sparse[PAGE(i2)][OFFSET(i2)] = j1;
  p->sparse[page][offset]         = TOMBSTONE;
  p->count--;
  // TODO: do some stuffs like sorting or grouping
  return UZU_TRUE;
}

void*
ecs_pool_set(ecs_Pool* p, ecs_entity_t ett, const void* data)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = PAGE(idx);
  u32 offset = OFFSET(idx);

  ecs_size_t* pagemem = assure_page(p->sparse, page);

  u32   j   = pagemem[offset];
  void* mem = NULL;
  if (j == TOMBSTONE)
  {
    mem = ecs_pool_add(p, ett);
    SDL_memcpy(mem, data, p->traits.size);
  }
  else
  {
    mem = (u8*)p->data + p->traits.size * j;
    SDL_memcpy(mem, data, p->traits.size);
    ecs_signal_emit(&p->signal[ECS_SIG_SET], ett, mem);
  }
  return mem;
}

void
ecs_pool_swp(ecs_Pool* p, ecs_entity_t ett1, ecs_entity_t ett2)
{
  u32 i1 = (ett1 >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 i2 = (ett2 >> ECS_ENT_IDX_SHIFT) & 0xffff;

  u32 page1   = PAGE(i1);
  u32 offset1 = OFFSET(i1);
  u32 page2   = PAGE(i2);
  u32 offset2 = OFFSET(i2);

  ecs_size_t j1 = p->sparse[page1][offset1];
  ecs_size_t j2 = p->sparse[page2][offset2];

  /* swap dense */
  p->entities[j1] = ett2;
  p->entities[j2] = ett1;
  dataswp(p->data, p->traits.size, j1, j2);

  /* swap sparse */
  p->sparse[page1][offset1] = j2;
  p->sparse[page2][offset2] = j1;
}

bool
ecs_pool_contains(ecs_Pool* p, ecs_entity_t ett)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = idx / PAGE_SIZ;
  u32 offset = idx % PAGE_SIZ;
  return (page < PAGE_CNT && p->sparse[page] != NULL &&
          p->sparse[page][offset] != TOMBSTONE);
}

ecs_size_t
ecs_pool_count(ecs_Pool* pool)
{
  return pool->count;
}

void
ecs_pool_data(ecs_Pool*            pool,
              void**               pData,
              const ecs_entity_t** pEtt,
              ecs_size_t*          pCnt)
{
  if (pData != NULL)
    *pData = pool->data;

  if (pEtt != NULL)
    *pEtt = pool->entities;

  if (pCnt != NULL)
    *pCnt = pool->count;
}

void
ecs_pool_connect(ecs_Pool* p, int sig, ecs_Callback callback, void* ctx)
{
  ecs_signal_connect(&p->signal[sig], callback, ctx);
}

void
ecs_pool_disconnect(ecs_Pool* p, int sig, ecs_Callback callback, void* ctx)
{
  ecs_signal_disconnect(&p->signal[sig], callback, ctx);
}

const ecs_entity_t*
ecs_pool_ett_rbegin(ecs_Pool* pool)
{
  return &pool->entities[pool->count - 1];
}

const ecs_entity_t*
ecs_pool_ett_rend(ecs_Pool* pool)
{
  return &pool->entities[-1];
}

void*
ecs_pool_data_rbegin(ecs_Pool* pool)
{
  return (u8*)pool->data + pool->traits.size * (pool->count - 1);
}

void*
ecs_pool_data_rend(ecs_Pool* pool)
{
  return (u8*)pool->data - pool->traits.size;
}

void
ecs_pool_take_ownership(ecs_Pool*   pool,
                        ecs_AddHook addHk,
                        ecs_RmvHook rmvHk,
                        void*       ctx)
{
  ASSERT(pool->addHook == NULL && pool->rmvHook == NULL);
  pool->addHook = addHk;
  pool->rmvHook = rmvHk;
  pool->hookCtx = ctx;
}

ecs_size_t
ecs_pool_index(ecs_Pool* p, ecs_entity_t ett)
{
  u32 idx    = (ett >> ECS_ENT_IDX_SHIFT) & 0xffff;
  u32 page   = PAGE(idx);
  u32 offset = OFFSET(idx);

  if (p->sparse[page] == NULL)
    return TOMBSTONE;
  return p->sparse[page][offset];
}

bool
ecs_pool_sortable(ecs_Pool* pool)
{
  return pool->addHook == NULL;
}
