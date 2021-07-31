#ifndef AABB_TREE_H
#define AABB_TREE_H
#include "common.h"

typedef struct RTree RTree;
#define RTREE_NULL_NODE (-1)

RTree* rtree_new();
void      rtree_delete(RTree* self);

int  rtree_create_proxy(RTree* self, void* user_data, const AABB* aabb);
void rtree_destroy_proxy(RTree* self, int proxy_id);
bool rtree_move_proxy(RTree* self, int proxy_id, const AABB* aabb, const Vec2 displacement);
void rtree_query(RTree* self, const AABB* aabb, Closure cb);

void* rtree_get_user_data(RTree* self, int proxy_id);

const AABB* rtree_get_fat_aabb(RTree* self, int proxy_id);

void rtree_draw_debug(RTree* self, SDL_Renderer* renderer, const SDL_Rect* viewport);

#endif // AABB_TREE_H
