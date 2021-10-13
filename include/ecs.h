// ngotrung Sun 18 Jul 2021 09:00:57 PM +07
#ifndef ECS_H
#define ECS_H
#include "toolbox.h"
#include <stdbool.h>
#include <stdint.h>

#define ECS_DEBUG

/* use enum to easier debug */
#ifndef ECS_TYPE_ID
#define ECS_TYPE_ID ecs_size_t
#endif

#ifndef ECS_MEM_FUNC
#define ECS_MEM_FUNC
#include <stdlib.h>
#define ECS_MALLOC(sz) malloc(sz)
#define ECS_REALLOC(ptr, newsz) realloc(ptr, newsz)
#define ECS_FREE(ptr) free(ptr)
#endif // ECS_MEM_FUNC

#ifndef ECS_ASSERT
#include <assert.h>
#define ECS_ASSERT(cond) assert(cond)
#endif // ECS_ASSERT

#define ECS_ASSERT_MSG(cond, msg) ASSERT((cond) && (msg))

#ifndef ECS_UNUSED
#define ECS_UNUSED
#endif // ECS_UNUSED

enum
{
    ECS_SIG_ADD, /* signal is emitted after component assigned */
    ECS_SIG_RMV, /* signal is emitted before component actually removed */
    ECS_SIG_SET, /* signal is emitted after component is set */
    ECS_SIG_CNT
};

typedef struct ecs_Registry ecs_Registry;
typedef struct ecs_Group    ecs_Group;

typedef uint32_t    ecs_entity_t;
typedef uint32_t    ecs_size_t;
typedef ECS_TYPE_ID ecs_type_id_t;

typedef void (*ecs_FiniFunc)(void*);
typedef void (*ecs_CpyFunc)(void*, const void*);

typedef void (*ecs_Callback)(void* ctx, ecs_Registry* registry, ecs_entity_t ett, void* mem);

struct ecs_TypeTraits
{
    /* component destructor */
    ecs_FiniFunc fini;

    /* component copy constructor */
    ecs_CpyFunc cpy;

    /* component size in byte */
    size_t size;

    /* component memory alignment */
    size_t align;

    const char* name;
};

#define ECS_IDX_MASK (0xffffu)
#define ECS_VER_MASK (0xffffu)

#define ECS_INDEX_SHIFT (0u)
#define ECS_VERSION_SHIFT (16u)
#define ECS_TO_INDEX(ett) (((ett) >> ECS_INDEX_SHIFT) & ECS_IDX_MASK)
#define ECS_TO_VERSION(ett) (((ett) >> ECS_VERSION_SHIFT) & ECS_VER_MASK)
#define ECS_TO_ENTITY(index, version)                                                                          \
    ((ecs_entity_t)(((index) << ECS_INDEX_SHIFT) | ((version) << ECS_VERSION_SHIFT)))

#define ECS_ENT_VER_SHIFT ECS_INDEX_SHIFT
#define ECS_ENT_IDX_SHIFT ECS_VERSION_SHIFT

#define ECS_SIZE_MAX 0xffffu
#define ECS_ENT_MAX 0xffffffffu

#define ECS_NULL_ENT ECS_ENT_MAX
#define ECS_NULL_IDX ECS_SIZE_MAX

#define ECS_ENT ECS_TO_ENTITY

#define ECS_EXCL_MASK 32768u
#define ECS_SHAR_MASK 4000u

#define ECS_TYPE_MASK 0xfffu

#define ECS_EXCL(X) ((X) | ECS_EXCL_MASK)
#define ECS_SHAR(X) ((X) | ECS_SHAR_MASK)

#ifndef ECS_PAGE_SIZE
#define ECS_PAGE_SIZE 128u
#endif
#ifndef ECS_PAGE_COUNT
#define ECS_PAGE_COUNT 64u
#endif

/* determine component data struct name relatived to it's identifier */
#ifndef ECS_COMP_NM
#define ECS_COMP_NM(T) struct T
#endif

#define IS_POWER_OF_TWO(x) ((x) && (((x) & ((x)-1)) == 0))

STATIC_ASSERT(IS_POWER_OF_TWO(ECS_PAGE_SIZE), ECS_PAGE_SIZE_MUST_BE_POWER_OF_TWO);
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
struct ecs_Registry* ecs_registry_create(const struct ecs_TypeTraits* types, ecs_size_t cnt);

/**
 * \brief Destroy given registry and all entities it manage
 *
 * \param reg
 */
void ecs_registry_free(struct ecs_Registry* reg);

/**
 * \brief Create new empty entity
 *
 * \param reg a registry
 * \return new empty entity
 */
ecs_entity_t ecs_create(struct ecs_Registry* reg);

/**
 * \brief Destory given entity and it's components if any
 *
 * \param reg a registry which manage given entity
 * \param ett a valid entity
 */
void ecs_destroy(struct ecs_Registry* reg, ecs_entity_t ett);

void* _ecs_add(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id);

/**
 * \brief Assign component to given entity. returned component is uninitialized
 *
 * \param reg a registry which manage given entity
 * \param ett a valid entity
 * \param type_id component type to be assigned to given componenet
 * \return memory of assigned component
 *
 * Do not keep reference of returned component because it can be jumping around
 * in internal array when same component type is assigned to or removed
 * from other entity.
 */
#define ecs_add(reg, ett, T) ((ECS_COMP_NM(T)*)_ecs_add(reg, ett, T))
#define ecs_addv(reg, ett, T, ...) (_ecs_addv(reg, ett, T, &(ECS_COMP_NM(T))__VA_ARGS__))
#define ecs_add_ex(reg, ett, T, ...) (_ecs_addv(reg, ett, T, &(ECS_COMP_NM(T))__VA_ARGS__))

void* _ecs_addv(struct ecs_Registry* reg, ecs_entity_t, ecs_type_id_t type_id, const void* data);

void* _ecs_set(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id, const void* data);

/**
 * \brief Set component data for given entity.
 *
 * \param reg registry which manage given entity
 * \param ett a valid entity
 * \param type_id component type to be set
 * \param data component data to set.
 * \return reference of updated component
 */
#define ecs_set(reg, ett, T, ...) ((ECS_COMP_NM(T)*)_ecs_set(reg, ett, T, &(ECS_COMP_NM(T))__VA_ARGS__))

void* _ecs_assurev(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id, const void* data);

#define ecs_assurev(reg, ett, T, ...) ((ECS_COMP_NM(T)*)_ecs_assurev(reg, ett, T, &(ECS_COMP_NM(T))__VA_ARGS__))

/**
 * \brief Remove comonent from an entity. Component destructor will be invoke
 *
 * \param reg registry which manage given entity
 * \param ett a valid entity
 * \param type_id component type to be removed
 */
void ecs_rmv(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id);

void ecs_rmv_all(struct ecs_Registry* reg, ecs_entity_t ett);

void* _ecs_get(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id);

#define ecs_get(reg, ett, T) ((ECS_COMP_NM(T)*)_ecs_get(reg, ett, T))

void* _ecs_assure(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id);

#define ecs_assure(reg, ett, T) ((ECS_COMP_NM(T)*)_ecs_assure(reg, ett, T))

void ecs_each(struct ecs_Registry* reg, ecs_Callback callback, void* ctx);

void ecs_raw(struct ecs_Registry* reg,
             ecs_size_t           ecs_type_id_t,
             const ecs_entity_t** ettPtr,
             void**               dataPtr,
             ecs_size_t*          cntPtr);

bool ecs_has(struct ecs_Registry* reg, ecs_entity_t ett, ecs_type_id_t type_id);

void ecs_clear(struct ecs_Registry* reg);

void
_ecs_getn(struct ecs_Registry* reg, ecs_entity_t ett, const ecs_type_id_t* types, ecs_size_t cnt, void* arr[]);

#define ecs_getn(reg, ett, out, ...)                                                                           \
    do                                                                                                         \
    {                                                                                                          \
        ecs_type_id_t Ts[] = __VA_ARGS__;                                                                      \
        ecs_getn(reg, ett, Ts, sizeof(Ts) / sizeof(ecs_type_id_t), out);                                       \
    } while (0)

ecs_entity_t ecs_cpy(struct ecs_Registry* dstReg, ecs_Registry* srcReg, ecs_entity_t srcEtt);

bool ecs_is_valid(struct ecs_Registry* reg, ecs_entity_t ett);

/**
 * \brief initialize a new entity group with given set of component types
 *
 * \param registry
 * \param Ts array of component type ids
 * \param Tc component type count
 */

ecs_Group* _ecs_create_group(ecs_Registry* registry, ecs_type_id_t* Ts, ecs_size_t Tc);

#define ecs_create_group(reg, ret, ...)                                                                        \
    do                                                                                                         \
    {                                                                                                          \
        ecs_size_t Ts[] = __VA_ARGS__;                                                                         \
        ecs_size_t Tc   = sizeof(Ts) / sizeof(ecs_size_t);                                                     \
        ret             = _ecs_create_group(reg, Ts, Tc);                                                      \
    } while (0)

void ecs_connect(struct ecs_Registry* reg, int event, ecs_type_id_t type_id, ecs_Callback callback, void* ctx);

void
ecs_disconnect(struct ecs_Registry* reg, int event, ecs_type_id_t type_id, ecs_Callback callback, void* ctx);

#define ECS_VIEW_MAX_COMP_CNT 16

/* actually this is iterator rather than a view? */
struct ecs_View
{
    u32                 fast_path;  /* fast path index                 */
    void*               data_iter;  /* fast path's data iterator       */
    void*               data_end;   /* end position                    */
    const ecs_entity_t* ett_iter;   /* fast path's entity iterator     */
    const ecs_entity_t* ett_end;    /* end position of ett iterator    */
    u32                 reqd_count; /* required component count        */
    u32                 excl_count; /* excluded component count        */
    size_t              tsiz;       /* fast path's component size      */

    /* reference to required pools */
    struct ecs_Pool* required_pools[ECS_VIEW_MAX_COMP_CNT];

    /* reference to excluded pools */
    struct ecs_Pool* excluded_pools[ECS_VIEW_MAX_COMP_CNT];
};

void _ecs_view_init(struct ecs_View* view, struct ecs_Registry* registry, ecs_type_id_t* types, u32 count);

#define ecs_view_init(v, reg, ...)                                                                             \
    do                                                                                                         \
    {                                                                                                          \
        ecs_type_id_t Ts[] = __VA_ARGS__;                                                                      \
        _ecs_view_init(v, reg, Ts, sizeof(Ts) / sizeof(ecs_type_id_t));                                        \
    } while (0)

bool _ecs_view_next(struct ecs_View* v, ecs_entity_t* ett, void* comps[]);

/* macro to shut up compiler warning */
#define ecs_view_next(view, ett, comps) _ecs_view_next(view, ett, (void**)comps)

void* ecs_group_data_begin(ecs_Group* group, ecs_size_t index);

const ecs_entity_t* ecs_group_ett_begin(ecs_Group* group);

ecs_size_t ecs_group_size(ecs_Group* group);

void ecs_group_each(ecs_Group* group, ecs_Callback callback, void* ctx);

#endif // ECS_H
