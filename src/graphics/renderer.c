#include "graphics/renderer.h"

struct BatchConfig
{
    enum DrawPrimitive primitive;
    int                priority;
    const Texture*     texture;
    const Material*    material;
    mat4               mvp_matrix;
};

struct Batch
{
    enum DrawPrimitive primitive;
    int                priority;
    const Texture*     texture;
    const Material*    material;
    mat4               mvp_matrix;
    struct Vertex*     vertcies;
    u32                size;
    u32                used;
    u32                vbo;
    u32                vao;
};

static struct Batch* s_batches;
static u32           s_batch_count;
static u32           s_used_batch_count;

static struct Batch*
get_next_availiable_batch(void)
{
    ASSERT_MSG(s_used_batch_count < s_batch_count, "No availiable batch");
    return &s_batches[s_used_batch_count++];
}

void
batch_init(struct Batch* batch, u32 size)
{
}
