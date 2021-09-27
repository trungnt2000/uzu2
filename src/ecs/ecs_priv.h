// ngotrung Thu 22 Jul 2021 06:45:55 PM +07
#ifndef ECS_PRIV_H
#define ECS_PRIV_H
#include "ecs.h"
#include "toolbox.h"

struct ecs_Pool;

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
typedef ecs_size_t (*ecs_AddHookFunc)(void* ctx, struct ecs_Pool* pool, ecs_entity_t ett, ecs_size_t idx);

/**
 * \brief Hook function to move component & entity to the end of the pool
 *        which invoke is hook function
 * \param ctx hook function contex
 * \param pool the pool which invoke this hook function
 * \param new component location
 */
typedef ecs_size_t (*ecs_RmvHookFunc)(void* ctx, struct ecs_Pool* pool, ecs_entity_t ett, ecs_size_t idx);

/**
 * \brief
 */
typedef ecs_size_t (
    *ecs_UpdtHookFunc)(void* ctx, struct ecs_Pool* pool, ecs_entity_t ett, ecs_size_t idx, const void* data);

struct ecs_Hook
{
    ecs_AddHookFunc  add;
    ecs_RmvHookFunc  rmv;
    ecs_UpdtHookFunc updt;
    void*            ctx;
};

#define ECS_HOOK_EMPTY ((struct ecs_Hook){ 0 })
#define ECS_HOOK(add_, rmv_, ctx_) ((ecs_Hook){ .add = add_, .rmv = rmv_, .ctx = ctx_ })

struct ecs_Registry
{
    const struct ecs_TypeTraits* type_traits;
    struct ecs_Pool**            pools;
    ecs_entity_t*                entities;
    ecs_size_t                   count;
    ecs_size_t                   size;
    ecs_size_t                   destroy_index;
    ecs_size_t                   type_count;
    ecs_size_t                   group_count;
    struct ecs_Group**           groups;
};

struct ecs_Slot
{
    ecs_Callback callback;
    void*        ctx;
};

struct ecs_Signal
{
    u32              size;
    u32              count;
    struct ecs_Slot* slots;
};

void ecs_signal_init(struct ecs_Signal* signal);
void ecs_signal_fini(struct ecs_Signal* signal);

void ecs_signal_connect(struct ecs_Signal* signal, ecs_Callback callback, void* ctx);

void ecs_signal_disconnect(struct ecs_Signal* signal, ecs_Callback callback, void* ctx);

void ecs_signal_emit(struct ecs_Signal* signal, struct ecs_Registry* registry, ecs_entity_t entity, void* mem);

struct ecs_Pool
{
    ecs_size_t*           sparse[ECS_PAGE_COUNT];
    ecs_entity_t*         entities;
    void*                 data_buffer;
    void*                 data;
    ecs_size_t            count;
    ecs_size_t            size;
    struct ecs_TypeTraits traits;
    struct ecs_Signal     signal[ECS_SIG_CNT];
    struct ecs_Hook       hook;
    struct ecs_Registry*  registry;
};

struct ecs_Pool*
ecs_pool_create(struct ecs_Registry* ecs_Registry, struct ecs_TypeTraits traits, ecs_size_t initialSize);

void ecs_pool_free(struct ecs_Pool* pool);

/**
 * add given entity to this pool also initialize it with given data
 */
void* ecs_pool_add(struct ecs_Pool* pool, ecs_entity_t ett, const void* data);

void* ecs_pool_add_or_set(struct ecs_Pool*, ecs_entity_t ett, const void* data);

/* get component memory associated with given entity if any
 * otherwise return NULL */
void* ecs_pool_get(struct ecs_Pool* pool, ecs_entity_t ett);

/* remove entity from this pool if any */
bool ecs_pool_rmv(struct ecs_Pool* pool, ecs_entity_t ett);

/* get raw access to this pool */
void ecs_pool_data(struct ecs_Pool* pool, void** pData, const ecs_entity_t** pEtt, ecs_size_t* pCnt);

/* set component data of given entity or add new one if this pool does not
 * contain given entity. Data is copy by memset */
void* ecs_pool_set(struct ecs_Pool* pool, ecs_entity_t ett, const void* data);

/* add given entity to this pool and invoke copy constructor if any */
void* ecs_pool_cpy(struct ecs_Pool* pool, ecs_entity_t ett, const void* other);

/* swap entity in packed array */
void ecs_pool_swp(struct ecs_Pool* pool, ecs_entity_t lhs, ecs_entity_t rhs);

/* get entity/component count */
ecs_size_t ecs_pool_count(struct ecs_Pool* pool);

/* chech whether or not this contains given entity */
bool ecs_pool_contains(struct ecs_Pool* pool, ecs_entity_t ett);

void ecs_pool_connect(struct ecs_Pool* pool, int signal, ecs_Callback callback, void* ctx);

const ecs_entity_t* ecs_pool_ett_rbegin(struct ecs_Pool* pool);
const ecs_entity_t* ecs_pool_ett_rend(struct ecs_Pool* pool);
void*               ecs_pool_data_rbegin(struct ecs_Pool* pool);
void*               ecs_pool_data_rend(struct ecs_Pool* pool);
void*               ecs_pool_data_begin(struct ecs_Pool* pool);
const ecs_entity_t* ecs_pool_ett_begin(struct ecs_Pool* pool);

void ecs_pool_disconnect(struct ecs_Pool* pool, int signal, ecs_Callback callback, void* ctx);

bool ecs_pool_sortable(struct ecs_Pool* pool);

void ecs_pool_set_hook(struct ecs_Pool* pool, struct ecs_Hook hk);

void ecs_pool_clear(struct ecs_Pool* pool);

/* get index of given entity */
ecs_size_t ecs_pool_index(struct ecs_Pool* p, ecs_entity_t ett);

struct ecs_Group
{
    struct ecs_Group* child;      /* reference to child group              */
    ecs_Registry*     registry;   /* reference to registry                 */
    struct ecs_Pool** own_pools;  /* reference to required component pools */
    ecs_size_t        own_count;  /* required component count              */
    struct ecs_Pool** excl_pools; /* reference excluded component pools    */
    ecs_size_t        excl_count; /* excluded component count              */
    struct ecs_Pool** shar_pools; /* reference to shared pools             */
    ecs_size_t        shar_count; /* shared component count                */
    ecs_size_t        size;       /* group size                            */
};

struct ecs_Group* ecs_group_create(struct ecs_Pool* const* master_pools, ecs_size_t* Ts, ecs_size_t Tc);

void ecs_group_free(struct ecs_Group* group);
#endif // ECS_PRIV_H
