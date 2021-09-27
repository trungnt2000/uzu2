#include "toolbox/rtree.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define RTREE_DEFAULT_CAP 512

typedef struct
{
    AABB  aabb;
    void* user_data;
    int   parent;
    int   child1;
    int   child2;
    int   height;
} TreeNode;

struct RTree
{
    int       size;
    int       count;
    TreeNode* nodes;
    int       root;
    int       free_index;
    int*      next;
};

static RTree*
rtree_init(RTree* t)
{
    t->size  = RTREE_DEFAULT_CAP;
    t->count = 0;
    t->root  = RTREE_NULL_NODE;
    t->nodes = calloc(RTREE_DEFAULT_CAP, sizeof(TreeNode));
    t->next  = calloc(RTREE_DEFAULT_CAP, sizeof(int));
    for (int i = 0; i < RTREE_DEFAULT_CAP - 1; ++i)
    {
        t->next[i]         = i + 1;
        t->nodes[i].height = -1;
    }
    t->free_index                  = 0;
    t->next[RTREE_DEFAULT_CAP - 1] = RTREE_NULL_NODE;
    return t;
}

static bool
node_is_leaf(const TreeNode* node)
{
    return node->child1 == -1;
}

static void
rtree_finalize(RTree* t)
{
    free(t->nodes);
    free(t->next);
}

static int
rtree_allocate_node(RTree* t)
{

    ASSERT(t->free_index != -1);

    TreeNode* nodes      = t->nodes;
    int       idx        = t->free_index;
    t->free_index        = t->next[idx];
    nodes[idx].user_data = NULL;
    nodes[idx].parent    = RTREE_NULL_NODE;
    nodes[idx].child1    = RTREE_NULL_NODE;
    nodes[idx].child2    = RTREE_NULL_NODE;
    nodes[idx].height    = 0;
    ++t->count;
    return idx;
}

static void
rtree_free_node(RTree* t, int node_idx)
{
    ASSERT(node_idx >= 0 && node_idx < t->size);
    ASSERT(t->count > 0);

    t->next[node_idx]         = t->free_index;
    t->free_index             = node_idx;
    t->nodes[node_idx].height = -1;
    --t->count;
}

static int
rtree_balance(RTree* t, int idx)
{
    int ia = idx;
    ASSERT(ia != RTREE_NULL_NODE);
    TreeNode* nodes = t->nodes;
    TreeNode* a     = &nodes[ia];
    if (node_is_leaf(a) || a->height < 2)
    {
        return ia;
    }

    int       ib      = a->child1;
    int       ic      = a->child2;
    TreeNode* b       = nodes + ib;
    TreeNode* c       = nodes + ic;
    int       balance = c->height - b->height;
    if (balance > 1)
    {
        int       ih = c->child1;
        int       ik = c->child2;
        TreeNode* h  = nodes + ih;
        TreeNode* k  = nodes + ik;

        c->child1 = ia;
        c->parent = a->parent;
        a->parent = ic;

        if (c->parent != RTREE_NULL_NODE)
        {
            if (nodes[c->parent].child1 == ia)
            {
                nodes[c->parent].child1 = ic;
            }
            else
            {
                nodes[c->parent].child2 = ic;
            }
        }
        else
        {
            t->root = ic;
        }

        if (h->height > k->height)
        {
            c->child2 = ih;
            a->child2 = ik;
            k->parent = ia;
            a->aabb   = aabb_merge(&b->aabb, &k->aabb);
            c->aabb   = aabb_merge(&a->aabb, &h->aabb);
            a->height = 1 + max(b->height, k->height);
            c->height = 1 + max(a->height, h->height);
        }
        else
        {
            c->child2 = ik;
            a->child2 = ih;
            h->parent = ia;
            a->aabb   = aabb_merge(&b->aabb, &h->aabb);
            c->aabb   = aabb_merge(&a->aabb, &k->aabb);
            a->height = 1 + max(b->height, h->height);
            c->height = 1 + max(a->height, k->height);
        }

        return ic;
    }

    if (balance < -1)
    {
        int       id = b->child1;
        int       ie = b->child2;
        TreeNode* d  = nodes + id;
        TreeNode* e  = nodes + ie;
        ASSERT(0 <= id && id < t->size);
        ASSERT(0 <= ie && ie < t->size);

        b->child1 = ia;
        b->parent = a->parent;
        a->parent = ib;

        if (b->parent != RTREE_NULL_NODE)
        {
            if (nodes[b->parent].child1 == ia)
            {
                nodes[b->parent].child1 = ib;
            }
            else
            {
                ASSERT(nodes[b->parent].child2 == ia);
                nodes[b->parent].child2 = ib;
            }
        }
        else
        {
            t->root = ib;
        }

        if (d->height > e->height)
        {
            b->child2 = id;
            a->child1 = ie;
            e->parent = ia;
            a->aabb   = aabb_merge(&c->aabb, &e->aabb);
            b->aabb   = aabb_merge(&a->aabb, &d->aabb);

            a->height = 1 + max(c->height, e->height);
            b->height = 1 + max(a->height, d->height);
        }
        else
        {
            b->child2 = ie;
            a->child1 = id;
            d->parent = ia;
            a->aabb   = aabb_merge(&c->aabb, &d->aabb);
            b->aabb   = aabb_merge(&a->aabb, &e->aabb);

            a->height = 1 + max(c->height, d->height);
            b->height = 1 + max(a->height, e->height);
        }

        return ib;
    }

    return ia;
}

static void
rtree_insert_leaf(RTree* t, int leaf)
{
    TreeNode* nodes = t->nodes;
    if (t->root == RTREE_NULL_NODE)
    {
        t->root               = leaf;
        nodes[t->root].parent = RTREE_NULL_NODE;
        return;
    }

    AABB leaf_aabb = nodes[leaf].aabb;
    int  index     = t->root;
    while (node_is_leaf(&nodes[index]) == false)
    {
        int child1 = nodes[index].child1;
        int child2 = nodes[index].child2;

        float area = aabb_premiter(&nodes[index].aabb);

        AABB  combined_aabb = aabb_merge(&nodes[index].aabb, &leaf_aabb);
        float combine_area  = aabb_premiter(&combined_aabb);

        float cost = 2.0f * combine_area;

        float inheritance_cost = 2.0f * (combine_area - area);

        float cost1;
        if (node_is_leaf(&nodes[child1]))
        {
            AABB aabb = aabb_merge(&leaf_aabb, &nodes[child1].aabb);
            cost1     = aabb_premiter(&aabb) + inheritance_cost;
        }
        else
        {
            AABB  aabb    = aabb_merge(&leaf_aabb, &nodes[child1].aabb);
            float oldArea = aabb_premiter(&nodes[child1].aabb);
            float newArea = aabb_premiter(&aabb);
            cost1         = (newArea - oldArea) + inheritance_cost;
        }

        float cost2;
        if (node_is_leaf(&nodes[child2]))
        {
            AABB aabb = aabb_merge(&leaf_aabb, &nodes[child2].aabb);
            cost2     = aabb_premiter(&aabb) + inheritance_cost;
        }
        else
        {
            AABB  aabb    = aabb_merge(&leaf_aabb, &nodes[child2].aabb);
            float oldArea = aabb_premiter(&nodes[child2].aabb);
            float newArea = aabb_premiter(&aabb);
            cost2         = newArea - oldArea + inheritance_cost;
        }

        if (cost < cost1 && cost < cost2)
        {
            break;
        }

        if (cost1 < cost2)
        {
            index = child1;
        }
        else
        {
            index = child2;
        }
    }

    int sibling = index;

    // Create a new parent.
    int old_parent              = nodes[sibling].parent;
    int new_parent              = rtree_allocate_node(t);
    nodes[new_parent].parent    = old_parent;
    nodes[new_parent].user_data = NULL;
    nodes[new_parent].aabb      = aabb_merge(&leaf_aabb, &nodes[sibling].aabb);
    nodes[new_parent].height    = nodes[sibling].height + 1;

    if (old_parent != RTREE_NULL_NODE)
    {
        // The sibling was not the root.
        if (nodes[old_parent].child1 == sibling)
        {
            nodes[old_parent].child1 = new_parent;
        }
        else
        {
            nodes[old_parent].child2 = new_parent;
        }

        nodes[new_parent].child1 = sibling;
        nodes[new_parent].child2 = leaf;
        nodes[sibling].parent    = new_parent;
        nodes[leaf].parent       = new_parent;
    }
    else
    {
        // The sibling was the root.
        nodes[new_parent].child1 = sibling;
        nodes[new_parent].child2 = leaf;
        nodes[sibling].parent    = new_parent;
        nodes[leaf].parent       = new_parent;
        t->root                  = new_parent;
    }

    // Walk back up the tree fixing heights and AABBs
    index = nodes[leaf].parent;
    while (index != RTREE_NULL_NODE)
    {
        index = rtree_balance(t, index);

        int child1 = nodes[index].child1;
        int child2 = nodes[index].child2;

        ASSERT(child1 != RTREE_NULL_NODE);
        ASSERT(child2 != RTREE_NULL_NODE);

        nodes[index].height = 1 + max(nodes[child1].height, nodes[child2].height);
        nodes[index].aabb   = aabb_merge(&nodes[child1].aabb, &nodes[child2].aabb);

        index = nodes[index].parent;
    }
}

static void
rtree_remove_leaf(RTree* t, int leaf)
{

    TreeNode* nodes = t->nodes;
    if (leaf == t->root)
    {
        t->root = RTREE_NULL_NODE;
        return;
    }

    int parent       = nodes[leaf].parent;
    int grand_parent = nodes[parent].parent;
    int sibling;
    if (nodes[parent].child1 == leaf)
    {
        sibling = nodes[parent].child2;
    }
    else
    {
        sibling = nodes[parent].child1;
    }

    if (grand_parent != RTREE_NULL_NODE)
    {
        if (nodes[grand_parent].child1 == parent)
        {
            nodes[grand_parent].child1 = sibling;
        }
        else
        {
            nodes[grand_parent].child2 = sibling;
        }
        nodes[sibling].parent = grand_parent;
        rtree_free_node(t, parent);

        int index = grand_parent;
        while (index != RTREE_NULL_NODE)
        {
            index = rtree_balance(t, index);

            int child1 = nodes[index].child1;
            int child2 = nodes[index].child2;

            nodes[index].aabb   = aabb_merge(&nodes[child1].aabb, &nodes[child2].aabb);
            nodes[index].height = 1 + max(nodes[child1].height, nodes[child2].height);

            index = nodes[index].parent;
        }
    }
    else
    {
        t->root               = sibling;
        nodes[sibling].parent = RTREE_NULL_NODE;
        rtree_free_node(t, parent);
    }
}

typedef struct
{
    int  capacity;
    int  count;
    int* items;
} Stack;

static void
stack_init(Stack* s)
{
    int n       = 64;
    s->capacity = n;
    s->count    = 0;
    s->items    = calloc(n, sizeof(int));
}

static void
stack_finalize(Stack* s)
{
    free(s->items);
}

static void
stack_grow(Stack* s)
{
    s->capacity *= 2;
    s->items = realloc(s->items, s->capacity * sizeof(int));
}

static void
stack_push(Stack* s, int v)
{
    if (s->count == s->capacity)
    {
        stack_grow(s);
    }

    s->items[s->count++] = v;
}
static int
stack_pop(Stack* s)
{
    ASSERT(s->count > 0 && "stack empty");
    return s->items[--s->count];
}
static bool
stack_empty(Stack* s)
{
    return s->count == 0;
}

/********************************************************************/

RTree*
rtree_new()
{
    return rtree_init(malloc(sizeof(RTree)));
}

void
rtree_delete(RTree* t)
{
    if (t)
    {
        rtree_finalize(t);
        free(t);
    }
}

int
rtree_create_proxy(RTree* t, void* user_data, const AABB* aabb)
{
    int       idx   = rtree_allocate_node(t);
    TreeNode* nodes = t->nodes;

    nodes[idx].aabb      = *aabb;
    nodes[idx].parent    = RTREE_NULL_NODE;
    nodes[idx].child1    = RTREE_NULL_NODE;
    nodes[idx].child2    = RTREE_NULL_NODE;
    nodes[idx].height    = 0;
    nodes[idx].user_data = user_data;

    aabb_extend(&nodes[idx].aabb, 5.f);

    rtree_insert_leaf(t, idx);

    return idx;
}

void
rtree_destroy_proxy(RTree* t, int proxy_id)
{
    ASSERT(0 <= proxy_id && proxy_id < t->size);
    ASSERT(node_is_leaf(&t->nodes[proxy_id]));

    rtree_remove_leaf(t, proxy_id);
    rtree_free_node(t, proxy_id);
}

bool
rtree_move_proxy(RTree* t, int proxy_id, const AABB* aabb, const Vec2 displacement)
{
    TreeNode* nodes = t->nodes;
    ASSERT(0 <= proxy_id && proxy_id < t->size);

    ASSERT(node_is_leaf(&nodes[proxy_id]));

    AABB fat_aabb = *aabb;
    aabb_extend(&fat_aabb, 7.f);

    Vec2 d = vec2_mul(displacement, 2.f);

    if (d.x < 0.0f)
    {
        fat_aabb.lower_bound.x += d.x;
    }
    else
    {
        fat_aabb.upper_bound.x += d.x;
    }

    if (d.y < 0.0f)
    {
        fat_aabb.lower_bound.y += d.y;
    }
    else
    {
        fat_aabb.upper_bound.y += d.y;
    }

    const AABB* tree_aabb = &nodes[proxy_id].aabb;
    if (aabb_contains(tree_aabb, aabb))
    {
        AABB huge_aabb = fat_aabb;
        aabb_extend(&huge_aabb, 4 * 5.f);

        if (aabb_contains(&huge_aabb, tree_aabb))
        {
            return false;
        }
    }

    rtree_remove_leaf(t, proxy_id);

    nodes[proxy_id].aabb = fat_aabb;

    rtree_insert_leaf(t, proxy_id);

    return true;
}

void
rtree_query(RTree* t, const AABB* aabb, bool (*cb)(void*, int), void* ctx)
{
    Stack     stack;
    TreeNode* nodes;

    nodes = t->nodes;
    stack_init(&stack);
    stack_push(&stack, t->root);
    while (!stack_empty(&stack))
    {
        int id = stack_pop(&stack);
        if (id == RTREE_NULL_NODE)
        {
            continue;
        }

        const TreeNode* node = nodes + id;
        if (aabb_test_overlap(aabb, &node->aabb))
        {
            if (node_is_leaf(node))
            {
                if (!cb(ctx, id))
                    break;
            }
            else
            {
                stack_push(&stack, node->child1);
                stack_push(&stack, node->child2);
            }
        }
    }
    stack_finalize(&stack);
}

void*
rtree_get_user_data(RTree* t, int proxy_id)
{
    ASSERT(proxy_id >= 0 && proxy_id < t->size);
    ASSERT(node_is_leaf(&t->nodes[proxy_id]));
    return t->nodes[proxy_id].user_data;
}

const AABB*
rtree_get_fat_aabb(RTree* t, int proxy_id)
{
    ASSERT(proxy_id >= 0 && proxy_id < t->size);
    ASSERT(node_is_leaf(&t->nodes[proxy_id]));
    return &t->nodes[proxy_id].aabb;
}
