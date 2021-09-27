#ifndef AABB_TREE_H
#define AABB_TREE_H
#include "common.h"

typedef struct
{
    float x;
    float y;
} Vec2;

typedef struct
{
    Vec2 lower_bound;
    Vec2 upper_bound;
} AABB;

INLINE Vec2
vec2_max(Vec2 a, Vec2 b)
{
    return (Vec2){ maxf(a.x, b.x), maxf(a.y, b.y) };
}

INLINE Vec2
vec2_min(Vec2 a, Vec2 b)
{
    return (Vec2){ minf(a.x, b.x), minf(a.y, b.y) };
}

INLINE Vec2
vec2_add(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x + b.x, a.y + b.y };
}

INLINE Vec2
vec2_sub(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x - b.x, a.y - b.y };
}

INLINE Vec2
vec2_mul(Vec2 a, float k)
{
    return (Vec2){ a.x * k, a.y * k };
}

INLINE bool
aabb_test_overlap(const AABB* a, const AABB* b)
{
    float dx1 = b->lower_bound.x - a->upper_bound.x;
    float dy1 = b->lower_bound.y - a->upper_bound.y;
    float dx2 = a->lower_bound.x - b->upper_bound.x;
    float dy2 = a->lower_bound.y - b->upper_bound.y;
    if (dx1 > 0.f || dy1 > 0.f)
        return false;
    if (dx2 > 0.f || dy2 > 0.f)
        return false;
    return true;
}

INLINE AABB
aabb_merge(const AABB* a, const AABB* b)
{
    return (AABB){
        .lower_bound = vec2_min(a->lower_bound, b->lower_bound),
        .upper_bound = vec2_max(a->upper_bound, b->upper_bound),
    };
}

INLINE float
aabb_premiter(const AABB* aabb)
{
    float w = aabb->upper_bound.x - aabb->lower_bound.x;
    float h = aabb->upper_bound.y - aabb->lower_bound.y;
    return 2.f * (w + h);
}

INLINE void
aabb_extend(AABB* aabb, float v)
{
    aabb->lower_bound.x -= v;
    aabb->lower_bound.y -= v;
    aabb->upper_bound.x += v;
    aabb->upper_bound.y += v;
}

INLINE bool
aabb_contains(const AABB* a, const AABB* b)
{
    bool result = true;
    result      = result && a->lower_bound.x <= b->lower_bound.x;
    result      = result && a->lower_bound.y <= b->lower_bound.y;
    result      = result && b->upper_bound.x <= a->upper_bound.x;
    result      = result && b->upper_bound.y <= a->upper_bound.y;
    return result;
}

INLINE int
rand_range(int min, int max)
{
    return rand() % (max - min) + min;
}

typedef struct RTree RTree;
#define RTREE_NULL_NODE (-1)

RTree* rtree_new();
void   rtree_delete(RTree* t);

int  rtree_create_proxy(RTree* t, void* user_data, const AABB* aabb);
void rtree_destroy_proxy(RTree* t, int proxy_id);
bool rtree_move_proxy(RTree* t, int proxy_id, const AABB* aabb, const Vec2 displacement);
void rtree_query(RTree* t, const AABB* aabb, bool(*cb)(void*, int), void* ctx);

void* rtree_get_user_data(RTree* t, int proxy_id);

const AABB* rtree_get_fat_aabb(RTree* t, int proxy_id);

#endif // AABB_TREE_H
