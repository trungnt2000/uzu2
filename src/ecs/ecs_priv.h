// ngotrung Thu 22 Jul 2021 06:45:55 PM +07
#ifndef ECS_PRIV_H
#define ECS_PRIV_H
#include "ecs.h"
#include "toolbox.h"

typedef struct ecs_Pool ecs_Pool;

/**
 * \brief Hook function to modify component location in internal array
 *        after entity have added in to the pool which invoke this hook
 *        function
 * \param ctx hook function context
 * \param pool the pool which invoke this function
 * \param ett newlly added entity
 * \param idx component index
 * \return new component location
 * */
typedef ecs_size_t (*ecs_AddHook)(void*        ctx,
                                  ecs_Pool*    pool,
                                  ecs_entity_t ett,
                                  ecs_size_t   idx);

/**
 * \brief Hook function to move component & entity to the end of the pool
 *        which invoke is hook function
 * \param ctx hook function contex
 * \param pool the pool which invoke this hook function
 * \param new component location
 */
typedef ecs_size_t (*ecs_RmvHook)(void*        ctx,
                                  ecs_Pool*    pool,
                                  ecs_entity_t ett,
                                  ecs_size_t   idx);

struct ecs_Registry
{
  const ecs_TypeTraits* typeTraits;
  ecs_Pool**            pools;
  ecs_entity_t*         entities;
  ecs_size_t            count;
  ecs_size_t            size;
  ecs_size_t            destroyIdx;
  ecs_size_t            typeCnt;
  ecs_size_t            groupCnt;
  ecs_Group**           groups;
};

typedef struct ecs_Slot
{
  ecs_Callback callback;
  void*        ctx;
} ecs_Slot;

typedef struct ecs_Signal
{
  u32       size;
  u32       count;
  ecs_Slot* slots;
} ecs_Signal;

void ecs_signal_init(ecs_Signal* signal);
void ecs_signal_fini(ecs_Signal* signal);

void ecs_signal_connect(ecs_Signal* signal, ecs_Callback callback, void* ctx);

void
ecs_signal_disconnect(ecs_Signal* signal, ecs_Callback callback, void* ctx);

void ecs_signal_emit(ecs_Signal* signal, ecs_entity_t entity, void* mem);

typedef struct ecs_Pool
{
  ecs_size_t*    sparse[ECS_SPARSE_SET_PAGE_CNT];
  ecs_entity_t*  entities;
  void*          dataBuffer;
  void*          data;
  ecs_size_t     count;
  ecs_size_t     size;
  ecs_TypeTraits traits;
  ecs_Signal     signal[ECS_SIG_CNT];
  ecs_AddHook    addHook;
  ecs_RmvHook    rmvHook;
  void*          hookCtx;
} ecs_Pool;

ecs_Pool* ecs_pool_create(ecs_TypeTraits traits, ecs_size_t initialSize);

void ecs_pool_free(ecs_Pool* pool);

/* add given entity to this pool and return uninitialized
 * component memory block */
void* ecs_pool_add(ecs_Pool* pool, ecs_entity_t ett);

/**
 * add given entity to this pool also initialize it with given data
 */
void* ecs_pool_add_ex(ecs_Pool* pool, ecs_entity_t ett, const void* data);

void* ecs_pool_add_or_set(ecs_Pool*, ecs_entity_t ett, const void* data);

/* get component memory associated with given entity if any
 * otherwise return NULL */
void* ecs_pool_get(ecs_Pool* pool, ecs_entity_t ett);

/* remove entity from this pool if any */
bool ecs_pool_rmv(ecs_Pool* pool, ecs_entity_t ett);

/* get raw access to this pool */
void ecs_pool_data(ecs_Pool*            pool,
                   void**               pData,
                   const ecs_entity_t** pEtt,
                   ecs_size_t*          pCnt);

/* set component data of given entity or add new one if this pool does not
 * contain given entity. Data is copy by memset */
void* ecs_pool_set(ecs_Pool* pool, ecs_entity_t ett, const void* data);

/* add given entity to this pool and invoke copy constructor if any */
void* ecs_pool_cpy(ecs_Pool* pool, ecs_entity_t ett, const void* other);

/* swap entity in packed array */
void ecs_pool_swp(ecs_Pool* pool, ecs_entity_t lhs, ecs_entity_t rhs);

/* get entity/component count */
ecs_size_t ecs_pool_count(ecs_Pool* pool);

/* chech whether or not this contains given entity */
bool ecs_pool_contains(ecs_Pool* pool, ecs_entity_t ett);

void
ecs_pool_connect(ecs_Pool* pool, int signal, ecs_Callback callback, void* ctx);

const ecs_entity_t* ecs_pool_ett_rbegin(ecs_Pool* pool);
const ecs_entity_t* ecs_pool_ett_rend(ecs_Pool* pool);
void*               ecs_pool_data_rbegin(ecs_Pool* pool);
void*               ecs_pool_data_rend(ecs_Pool* pool);
void*               ecs_pool_data_begin(ecs_Pool* pool);
const ecs_entity_t* ecs_pool_ett_begin(ecs_Pool* pool);

void ecs_pool_disconnect(ecs_Pool*    pool,
                         int          signal,
                         ecs_Callback callback,
                         void*        ctx);

bool ecs_pool_sortable(ecs_Pool* pool);

void ecs_pool_take_ownership(ecs_Pool*   pool,
                             ecs_AddHook addHk,
                             ecs_RmvHook rmvHk,
                             void*       ctx);

/* get index of given entity */
ecs_size_t ecs_pool_index(ecs_Pool* p, ecs_entity_t ett);
#endif // ECS_PRIV_H
