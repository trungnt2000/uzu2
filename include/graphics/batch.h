// ngotrung Thu 30 Sep 2021 02:09:56 PM +07
#ifndef GRAPHICS_BATCH
#define GRAPHICS_BATCH
#include "cglm/cglm.h"
#include "graphics/types.h"

typedef struct Batch Batch;

void batches_init(u32 batch_count, u32 batch_size);
void begin_draw(void);
void end_draw(void);

Batch* next_free_batch(void);

void batch_add(Batch* batch, const struct Vertex* vertices, u32 vertex_count);


#endif // GRAPHICS_BATCH
