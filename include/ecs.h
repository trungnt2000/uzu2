// ngotrung Sun 18 Jul 2021 09:00:57 PM +07
#ifndef ECS_H
#define ECS_H
#include "toolbox.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef ECS_ENTITY_TYPE
#define ECS_ENTITY_TYPE uint32_t
#endif

#ifndef ECS_SIZE_TYPE
#define ECS_SIZE_TYPE uint16_t
#endif

/* use enum to easier debug */
#ifndef ECS_COMPONENT_IDENTIFIER_TYPE
#define ECS_COMPONENT_IDENTIFIER_TYPE ecs_size_t
#endif

enum
{
  ECS_SIG_ADD, /* signal is emitted after component assigned */
  ECS_SIG_RMV, /* signal is emitted before component actually removed */
  ECS_SIG_SET, /* signal is emitted after component is set */
  ECS_SIG_CNT
};

typedef ECS_ENTITY_TYPE               ecs_entity_t;
typedef ECS_SIZE_TYPE                 ecs_size_t;
typedef ECS_COMPONENT_IDENTIFIER_TYPE ecs_comp_id_t;
typedef struct ecs_Pool               ecs_Pool;
typedef struct ecs_Registry           ecs_Registry;

typedef void (*ecs_InitFunc)(void*);
typedef void (*ecs_FiniFunc)(void*);
typedef void (*ecs_CpyFunc)(void*, const void*);
typedef int (*ecs_CmpFunc)(const void*, const void*);

typedef void (*ecs_Callback)(void* ctx, ecs_entity_t ett, void* mem);
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

typedef struct ecs_TypeTraits
{
  /* component constructor */
  ecs_InitFunc init;

  /* component destructor */
  ecs_FiniFunc fini;

  /* component copy constructor */
  ecs_CpyFunc cpy;

  /* component compare function */
  ecs_CmpFunc cmp;

  /* component size in byte */
  size_t size;

  /* component memory alignment */
  size_t align;
} ecs_TypeTraits;

#define ECS_ENT_VER_SHIFT 16
#define ECS_ENT_IDX_SHIFT 0
#define ECS_SIZE_MAX 0xffff
#define ECS_ENT_MAX 0xffffffff

#define ECS_NULL_ENT ECS_ENT_MAX
#define ECS_NULL_IDX ECS_SIZE_MAX

#define ECS_IDX_MASK 0xffff
#define ECS_VER_MASK 0xffff

#define ECS_ENT_IDX(e) (((e) >> ECS_ENT_IDX_SHIFT) & ECS_IDX_MASK)
#define ECS_ENT_VER(e) (((e) >> ECS_ENT_VER_SHIFT) & ECS_VER_MASK)
#define ECS_ENT(idx, ver)                                                      \
  (((idx) << ECS_ENT_IDX_SHIFT) | ((ver) << ECS_ENT_VER_SHIFT))

#define ECS_ENT_FMT_PATTERN "[%4u | %4u]"
#define ECS_ENT_FMT_VARS(ent) ECS_ENT_IDX(ent), ECS_ENT_VER(ent)

#define ECS_EXCL_MASK 32768u
#define ECS_SHARED_MASK 4000u

#define ECS_TYPE_MASK 0xfff

#define ECS_EXCL(X) ((X) | ECS_EXCL_MASK)
#define ECS_SHARED(X) ((X) | ECS_SHARED_MASK)

#ifndef ECS_SPARSE_SET_PAGE_SIZ
#define ECS_SPARSE_SET_PAGE_SIZ 128
#endif
#ifndef ECS_SPARSE_SET_PAGE_CNT
#define ECS_SPARSE_SET_PAGE_CNT 64
#endif

#ifndef ECS_COMP_NM
#define ECS_COMP_NM(T) _##T
#endif

#define IS_POWER_OF_TWO(x) ((x) && (((x) & ((x)-1)) == 0))

STATIC_ASSERT(IS_POWER_OF_TWO(ECS_SPARSE_SET_PAGE_SIZ),
              ECS_SPARSE_SET_PAGE_SIZ_IS_POWER_OF_TWO);
/**
 * \brief Struct keep track all entities we created
 */
typedef struct ecs_Registry ecs_Registry;

/**
 * \brief Create new registry
 * \param types array of ecs_TypeTraits describe all component in our game,
 *        registry will not copy given types array
 * \param cnt how many component types
 * \return new registry
 */
ecs_Registry* ecs_registry_create(const ecs_TypeTraits* types, ecs_size_t cnt);

/**
 * \brief Destroy given registry and all entities it manage
 *
 * \param reg
 */
void ecs_registry_free(ecs_Registry* reg);

/**
 * \brief Create new entity with no component
 *
 * \param reg a registry
 * \return new empty entity
 */
ecs_entity_t ecs_create(ecs_Registry* reg);

/**
 * \brief Destory given entity and it's components if any
 *
 * \param reg a registry which manage given entity
 * \param ett a valid entity
 */
void ecs_destroy(ecs_Registry* reg, ecs_entity_t ett);

/**
 * \brief Assign component to given entity. Component's constructor will be
 *        invoke.
 *
 * \param reg a registry which manage given entity
 * \param ett a valid entity
 * \param typeId component type to be assigned to given componenet
 * \param optional component data
 * \return memory of assigned component
 *
 * Do not keep reference of returned component because it can be jumping around
 * in internal array when same component type is assigned to or removed
 * from other entity.
 */
void* _ecs_add(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId);

/**
 * Macro wrap around _ecs_add function to cast return value to
 * to given type T
 */
#define ecs_add(reg, ett, T) ((ECS_COMP_NM(T)*)_ecs_add(reg, ett, T))

/**
 * \brief Remove comonent from an entity. Component destructor will be invoke
 *
 * \param reg registry which manage given entity
 * \param ett a valid entity
 * \param typeId component type to be removed
 */
void ecs_rmv(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId);

/**
 * \brief Set component data for given entity, if given entity does not have
 *        given type it will be assigned that component.Component data will be
 *        copied by memcpy.
 *
 * \param reg registry which manage given entity
 * \param ett a valid entity
 * \param typeId component type to be set
 * \param data component data to set.
 * \return reference of updated component
 */
void* _ecs_set(ecs_Registry* reg,
               ecs_entity_t  ett,
               ecs_size_t    typeId,
               const void*   data);

#define ecs_set(reg, ett, T, ...)                                              \
  ((ECS_COMP_NM(T)*)_ecs_set(reg, ett, T, &(ECS_COMP_NM(T))__VA_ARGS__))

void ecs_rmv_all(ecs_Registry* reg, ecs_entity_t ett);

void* _ecs_get(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId);

#define ecs_get(reg, ett, T) ((ECS_COMP_NM(T)*)_ecs_get(reg, ett, T))

void ecs_each(ecs_Registry* reg, ecs_Callback callback, void* ctx);

void ecs_raw(ecs_Registry*        reg,
             ecs_size_t           typeId,
             const ecs_entity_t** pEttArr,
             void**               pDataArr,
             ecs_size_t*          pCnt);

bool ecs_has(ecs_Registry* reg, ecs_entity_t ett, ecs_size_t typeId);

void ecs_clear(ecs_Registry* reg);

void _ecs_getn(ecs_Registry*     reg,
               ecs_entity_t      ett,
               const ecs_size_t* types,
               ecs_size_t        cnt,
               void*             arr[]);

#define ecs_getn(reg, ett, out, ...)                                           \
  do                                                                           \
  {                                                                            \
    ecs_size_t __Ts[] = __VA_ARGS__;                                           \
    ecs_getn(reg, ett, __Ts, sizeof(__Ts) / sizeof(ecs_size_t), out);          \
  } while (0)

ecs_entity_t
ecs_cpy(ecs_Registry* dstReg, ecs_Registry* srcReg, ecs_entity_t srcEtt);

bool ecs_is_valid(ecs_Registry* reg, ecs_entity_t ett);

void ecs_connect(ecs_Registry* reg,
                 int           event,
                 ecs_size_t    typeId,
                 ecs_Callback  callback,
                 void*         ctx);

void ecs_disconnect(ecs_Registry* reg,
                    int           event,
                    ecs_size_t    typeId,
                    ecs_Callback  callback,
                    void*         ctx);

#define ECS_VIEW_MAX_COMP_CNT 16

/* actually this is iterator rather than a view? */
typedef struct ecs_View
{
  u32                 fastPath; /* fast path index                 */
  void*               dataIter; /* fast path's data iterator       */
  void*               dataEnd;  /* end position                    */
  const ecs_entity_t* ettIter;  /* fast path's entity iterator     */
  const ecs_entity_t* ettEnd;   /* end position of ett iterator    */
  u32                 reqdCnt;  /* required component count        */
  u32                 exclCnt;  /* excluded component count        */
  size_t              tsiz;     /* fast path's component size      */

  /* reference to required pools */
  ecs_Pool* reqdPools[ECS_VIEW_MAX_COMP_CNT];

  /* reference to excluded pools */
  ecs_Pool* exclPools[ECS_VIEW_MAX_COMP_CNT];
} ecs_View;

void _ecs_view_init(ecs_View*     view,
                    ecs_Registry* registry,
                    ecs_size_t*   types,
                    u32           count);

#define ecs_view_init(v, reg, ...)                                             \
  do                                                                           \
  {                                                                            \
    ecs_size_t __Ts[] = __VA_ARGS__;                                           \
    _ecs_view_init(v, reg, __Ts, sizeof(__Ts) / sizeof(ecs_size_t));           \
  } while (0)

bool _ecs_view_next(ecs_View* v, ecs_entity_t* ett, void* comps[]);

/* macro to shut up compiler warning */
#define ecs_view_next(view, ett, comps) _ecs_view_next(view, ett, (void**)comps)

typedef struct ecs_Group
{
  struct ecs_Group* child;     /* reference to child group              */
  ecs_Registry*     registry;  /* reference to registry                 */
  ecs_Pool**        ownPools;  /* reference to required component pools */
  ecs_size_t        ownCnt;    /* required component count              */
  ecs_Pool**        exclPools; /* reference excluded component pools    */
  ecs_size_t        exclCnt;   /* excluded component count              */
  ecs_Pool**        sharPools; /* reference to shared pools             */
  ecs_size_t        sharCnt;   /* shared component count                */
  ecs_size_t        size;      /* group size                            */
} ecs_Group;

/**
 * \brief initialize a new entity group with given set of component types
 *
 * \param registry
 * \param Ts array of component type ids
 * \param Tc component type count
 */
void _ecs_group_init(ecs_Group*    group,
                     ecs_Registry* registry,
                     ecs_size_t*   Ts,
                     ecs_size_t    Tc);

/**
 *  \biref _ecs_group_init's wrapper macro
 *
 *  Ex:
 *    ecs_Group group;
 *    ecs_group_init(&group, registry, { Velocity, Position })
 */
#define ecs_group_init(group, registry, ...)                                   \
  do                                                                           \
  {                                                                            \
    ecs_size_t __Ts[] = __VA_ARGS__;                                           \
    ecs_size_t __Tc   = sizeof(__Ts) / sizeof(ecs_size_t);                     \
    _ecs_group_init(group, registry, __Ts, __Tc);                              \
  } while (0)

void ecs_group_destroy(ecs_Group* group);

void* ecs_group_data_begin(ecs_Group* group, ecs_size_t index);

const ecs_entity_t* ecs_group_ett_begin(ecs_Group* group);

ecs_size_t ecs_group_size(ecs_Group* group);

void ecs_group_each(ecs_Group* group, ecs_Callback callback, void* ctx);
#endif // ECS_H
