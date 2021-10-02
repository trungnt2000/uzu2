// ngotrung Mon 13 Sep 2021 06:49:03 PM +07
#ifndef QUAD_TREE_H
#define QUAD_TREE_H
#include "toolbox/common.h"

struct Box
{
    float left;
    float top;
    float right;
    float bottom;
};

typedef struct QuadTree QuadTree;

#define QUAD_TREE_NULL_ID (-1)

QuadTree* quad_tree_create(int depth_limit, struct Box primary_box);

void quad_tree_free(QuadTree* tree);

#define quad_tree_free_null(t)                                                                                 \
    do                                                                                                         \
    {                                                                                                          \
        quad_tree_free(t);                                                                                     \
        t = NULL;                                                                                              \
    } while (0)

int quad_tree_create_element(QuadTree* tree, struct Box box, void* user_data);

void quad_tree_destroy_element(QuadTree* tree, int element_id);

void quad_tree_update_element(QuadTree* tree, int element_id, struct Box box);

void quad_tree_query(QuadTree* tree, struct Box box, bool(void*, int), void* ctx);

void* quad_tree_get_user_data(QuadTree* tree, int element_id);

struct Box quad_tree_get_bounding_box(QuadTree* tree, int element_id);

void quad_tree_foreach_node(QuadTree* tree, bool(void*, struct Box), void* ctx);

void quad_tree_foreach_element(QuadTree* tree, bool(void*, struct Box), void* ctx);

#endif // QUAD_TREE_H
