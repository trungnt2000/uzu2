#include "toolbox/quad_tree.h"
#include "toolbox/common.h"

#define NULL_INDEX QUAD_TREE_NULL_ID
#define NODE_PER_BLOCK (4)

struct FreeList
{
    void*  data;
    size_t element_size;
    size_t element_aligment;
    int    first_free_index;
    int*   next;
    int    count;
    int    size;
};

void
free_list_resize(struct FreeList* lst, int size)
{
    lst->size = size;
    lst->next = SDL_realloc(lst->next, (sizeof *lst->next) * (size_t)size);
    lst->data = SDL_realloc(lst->data, lst->element_size * (size_t)size);

    for (int i = lst->count; i < lst->size - 1; ++i)
    {
        lst->next[i] = i + 1;
    }
    lst->next[lst->size - 1] = NULL_INDEX;
    lst->first_free_index    = lst->count;
}

struct FreeList*
free_list_init(struct FreeList* lst, size_t element_size, size_t element_aligment)
{
    lst->element_size     = element_size;
    lst->element_aligment = element_aligment;
    lst->count            = 0;
    lst->data             = NULL;
    lst->next             = NULL;

    free_list_resize(lst, 16);

    return (lst);
}

void
free_list_destroy(struct FreeList* lst)
{
    if (lst != NULL)
    {
        SDL_free(lst->data);
        lst->data = NULL;
        SDL_free(lst->next);
        lst->next = NULL;
    }
}

void*
free_list_at(struct FreeList* lst, int index)
{
    ASSERT(index >= 0 && index < lst->size);
    return (u8*)lst->data + lst->element_size * (size_t)index;
}

int
free_list_alloc_elmement(struct FreeList* lst)
{
    if (lst->count == lst->size)
    {
        ASSERT_MSG(lst->first_free_index == NULL_INDEX, "Some thing wrong");
        free_list_resize(lst, lst->size * 2);
    }

    int new_index         = lst->first_free_index;
    lst->first_free_index = lst->next[new_index];
    lst->count++;

    return new_index;
}

void
free_list_free_element(struct FreeList* lst, int index)
{
    ASSERT(index >= 0 && index < lst->size);
    lst->next[index]      = lst->first_free_index;
    lst->first_free_index = index;
    lst->count--;
}

void*
free_list_firstf(struct FreeList* lst)
{
    return lst->data;
}

int
free_list_range(struct FreeList* lst)
{
    return lst->size;
}

#define free_list_first(T, lst) ((T*)free_list_firstf(lst))

struct Element
{
    struct Box bounds;
    void*      user_data;
    int        node_id;
    int        next;
    int        prev;
};

struct Node
{
    int count;
    int first_element;
    int first_child;
    int parent;
};

struct NodeData
{
    int        node_id;
    struct Box bounds;
};

struct Stack
{
    struct NodeData* elements;
    int              sp;
    int              max;
};

struct QuadTree
{
    struct FreeList elememts;
    struct FreeList nodes;
    struct Box      bounds;
    int             root;
    int             depth_limit;
};

void
stack_init(struct Stack* st, int max)
{
    st->elements = SDL_malloc((size_t)max * (sizeof *st->elements));
    st->max      = max;
    st->sp       = 0;
}

void
stack_destroy(struct Stack* st)
{
    SDL_free(st->elements);
    st->elements = NULL;
}

static void
stack_push(struct Stack* st, const struct NodeData element)
{
    if (st->sp == st->max)
    {
        st->max = st->max * 2;
        SDL_realloc(st->elements, (sizeof *st->elements) * (size_t)st->max);
    }
    st->elements[st->sp++] = element;
}

static struct NodeData
stack_pop(struct Stack* st)
{
    return st->elements[--st->sp];
}

static bool
stack_is_empty(struct Stack* st)
{
    return st->sp == 0;
}

bool
check_box_contains(const struct Box* b1, const struct Box* b2)
{
    return b1->left <= b2->left && b1->right >= b2->right && b1->top <= b2->top && b1->bottom >= b2->bottom;
}

bool
check_box_overlaps(const struct Box* b1, const struct Box* b2)
{
    bool hori_overlap =
        (b1->left >= b2->left && b1->left <= b2->right) || (b2->left >= b1->left && b2->left <= b1->right);

    bool vert_overlap =
        (b1->top >= b2->top && b1->top <= b2->bottom) || (b2->top >= b1->top && b2->top <= b1->bottom);

    return (hori_overlap && vert_overlap);
}

void
extend_box(struct Box* b, float v)
{
    b->left -= v;
    b->top -= v;
    b->bottom += v;
    b->right += v;
}

int quad_tree_create_node(struct QuadTree* tree);

struct QuadTree*
quad_tree_create(int depth_limit, struct Box bounds)
{
    struct QuadTree* tree = SDL_malloc(sizeof *tree);

    free_list_init(&tree->elememts, sizeof(struct Element), _Alignof(struct Element));
    free_list_init(&tree->nodes, sizeof(struct Node) * NODE_PER_BLOCK, _Alignof(struct Node));
    tree->bounds      = bounds;
    tree->depth_limit = depth_limit;
    tree->root        = quad_tree_create_node(tree);

    return (tree);
}

void
quad_tree_free(QuadTree* tree)
{
    if (tree != NULL)
    {
        free_list_destroy(&tree->nodes);
        free_list_destroy(&tree->elememts);
    }
}

int
quad_tree_create_node(struct QuadTree* tree)
{
    int          block_index = free_list_alloc_elmement(&tree->nodes);
    struct Node* node        = free_list_at(&tree->nodes, block_index);

    // allocate four node at once
    node[0].first_child   = NULL_INDEX;
    node[0].first_element = NULL_INDEX;
    node[0].count         = 0;
    node[0].parent        = NULL_INDEX;

    node[1].first_element = NULL_INDEX;
    node[1].first_child   = NULL_INDEX;
    node[1].count         = 0;
    node[1].parent        = NULL_INDEX;

    node[2].first_child   = NULL_INDEX;
    node[2].first_element = NULL_INDEX;
    node[2].count         = 0;
    node[2].parent        = NULL_INDEX;

    node[3].first_child   = NULL_INDEX;
    node[3].first_element = NULL_INDEX;
    node[3].count         = 0;
    node[3].parent        = NULL_INDEX;

    return block_index * NODE_PER_BLOCK;
}

static void
quad_tree_free_node(struct QuadTree* tree, int index)
{
    free_list_free_element(&tree->nodes, index);
}

static struct Node*
quad_tree_get_node(struct QuadTree* tree, int node_id)
{
    ASSERT_MSG(node_id >= 0 && node_id < free_list_range(&tree->nodes) * NODE_PER_BLOCK,
               "Invalid node_id(out of rage)");
    return &((struct Node*)tree->nodes.data)[node_id];
}

static int
quad_tree_alloc_element(struct QuadTree* tree)
{
    int             element_id = free_list_alloc_elmement(&tree->elememts);
    struct Element* element    = free_list_at(&tree->elememts, element_id);

    element->next      = NULL_INDEX;
    element->prev      = NULL_INDEX;
    element->node_id   = NULL_INDEX;
    element->user_data = NULL;
    return element_id;
}

static void
quad_tree_free_element(struct QuadTree* tree, int element_id)
{
    free_list_free_element(&tree->elememts, element_id);
}

static struct Element*
quad_tree_get_element(struct QuadTree* tree, int element_id)
{
    ASSERT_MSG(element_id >= 0 && element_id < free_list_range(&tree->elememts),
               "Invalid element_id(out of range)");
    return &free_list_first(struct Element, &tree->elememts)[element_id];
}

static int
quad_tree_assure_child(struct QuadTree* tree, int node_id, int child_index)
{
    struct Node* node = quad_tree_get_node(tree, node_id);
    if (node->first_child == NULL_INDEX)
    {
        int new_node = quad_tree_create_node(tree);

        // memory can be reallocated
        node              = quad_tree_get_node(tree, node_id);
        node->first_child = new_node;

        struct Node* child = quad_tree_get_node(tree, new_node);

        child[0].parent = node_id;
        child[1].parent = node_id;
        child[2].parent = node_id;
        child[3].parent = node_id;
    }
    return node->first_child + child_index;
}

void
quad_tree_insert_element(struct QuadTree* tree, int element_id)
{
    int        node_id        = tree->root;
    int        depth          = 0;
    bool       found          = false;
    struct Box element_bounds = quad_tree_get_element(tree, element_id)->bounds;
    struct Box node_bounds    = tree->bounds;

    while (!found)
    {
        if (depth >= tree->depth_limit)
            found = true;

        float m0 = (node_bounds.left + node_bounds.right) / 2.f;
        float m1 = (node_bounds.top + node_bounds.bottom) / 2.f;

        struct Box TL = { node_bounds.left, node_bounds.top, m0, m1 };
        struct Box TR = { m0, node_bounds.top, node_bounds.right, m1 };
        struct Box BL = { node_bounds.left, m1, m0, node_bounds.bottom };
        struct Box BR = { m0, m1, node_bounds.right, node_bounds.bottom };

        if (check_box_contains(&TL, &element_bounds))
        {
            node_id     = quad_tree_assure_child(tree, node_id, 0);
            node_bounds = TL;
            depth++;
        }
        else if (check_box_contains(&TR, &element_bounds))
        {
            node_id     = quad_tree_assure_child(tree, node_id, 1);
            node_bounds = TR;
            depth++;
        }
        else if (check_box_contains(&BL, &element_bounds))
        {
            node_id     = quad_tree_assure_child(tree, node_id, 2);
            node_bounds = BL;
            depth++;
        }
        else if (check_box_contains(&BR, &element_bounds))
        {
            node_bounds = BR;
            node_id     = quad_tree_assure_child(tree, node_id, 3);
            depth++;
        }
        else
        {
            found = true;
        }
    }

    struct Node*    node    = quad_tree_get_node(tree, node_id);
    struct Element* element = quad_tree_get_element(tree, element_id);

    if (node->first_element != NULL_INDEX)
    {
        struct Element* first_element = quad_tree_get_element(tree, node->first_element);
        first_element->prev           = element_id;
    }

    element->next    = node->first_element;
    element->prev    = NULL_INDEX;
    element->node_id = node_id;

    node->first_element = element_id;
    node->count++;
}

void
quad_tree_query(struct QuadTree* tree, struct Box box, bool (*callback)(void*, int), void* ctx)
{
    struct Stack stack = { 0 };

    struct Node*    nodes    = tree->nodes.data;
    struct Element* elements = tree->elememts.data;

    struct NodeData data;

    stack_init(&stack, 128);
    stack_push(&stack, (struct NodeData){ tree->root, tree->bounds });

    while (!stack_is_empty(&stack))
    {
        data    = stack_pop(&stack);
        int eid = nodes[data.node_id].first_element;

        for (; eid != NULL_INDEX; eid = elements[eid].next)
            if (check_box_overlaps(&elements[eid].bounds, &box))
            {
                if (!callback(ctx, eid))
                {
                    goto cleanup;
                }
            }

        int fc = nodes[data.node_id].first_child;

        if (fc != NULL_INDEX)
        {
            struct Box bounds = data.bounds;

            float m0 = (bounds.left + bounds.right) / 2.f;
            float m1 = (bounds.top + bounds.bottom) / 2.f;

            struct Box TL = { bounds.left, bounds.top, m0, m1 };
            struct Box TR = { m0, bounds.top, bounds.right, m1 };
            struct Box BL = { bounds.left, m1, m0, bounds.bottom };
            struct Box BR = { m0, m1, bounds.right, bounds.bottom };

            if (check_box_overlaps(&TL, &box))
                stack_push(&stack, (struct NodeData){ fc + 0, TL });

            if (check_box_overlaps(&TR, &box))
                stack_push(&stack, (struct NodeData){ fc + 1, TR });

            if (check_box_overlaps(&BL, &box))
                stack_push(&stack, (struct NodeData){ fc + 2, BL });

            if (check_box_overlaps(&BR, &box))
                stack_push(&stack, (struct NodeData){ fc + 3, BR });
        }
    }

cleanup:
    stack_destroy(&stack);
}

void
quad_tree_remove_element(struct QuadTree* tree, int element_id)
{
    struct Element* elements = quad_tree_get_element(tree, 0);

    struct Node* node = quad_tree_get_node(tree, elements[element_id].node_id);

    // delete head
    if (element_id == node->first_element)
        node->first_element = elements[element_id].next;

    int next = elements[element_id].next;
    int prev = elements[element_id].prev;

    if (next != NULL_INDEX)
        elements[next].prev = prev;

    if (prev != NULL_INDEX)
        elements[prev].next = next;

    elements[element_id].next    = NULL_INDEX;
    elements[element_id].prev    = NULL_INDEX;
    elements[element_id].node_id = NULL_INDEX;
    node->count--;
}

void
quad_tree_update_element(struct QuadTree* tree, int element_id, struct Box box)
{
    struct Element* element = quad_tree_get_element(tree, element_id);

    if (check_box_contains(&element->bounds, &box))
        return;

    extend_box(&box, 5.f);
    element->bounds = box;
    quad_tree_remove_element(tree, element_id);
    quad_tree_insert_element(tree, element_id);
}

int
quad_tree_create_element(struct QuadTree* tree, struct Box box, void* user_data)
{
    int             element_id = quad_tree_alloc_element(tree);
    struct Element* element    = quad_tree_get_element(tree, element_id);
    element->bounds            = box;
    element->user_data         = user_data;
    quad_tree_insert_element(tree, element_id);
    return element_id;
}

void
quad_tree_destroy_element(struct QuadTree* tree, int element_id)
{
    quad_tree_remove_element(tree, element_id);
    quad_tree_free_element(tree, element_id);
}

void*
quad_tree_get_user_data(struct QuadTree* tree, int element_id)
{
    const struct Element* element = quad_tree_get_element(tree, element_id);

    return (element->user_data);
}

struct Box
quad_tree_get_bounding_box(QuadTree* tree, int element_id)
{
    const struct Element* element = quad_tree_get_element(tree, element_id);

    return (element->bounds);
}
