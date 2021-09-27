//ngotrung Sat 17 Jul 2021 09:27:07 PM +07
#ifndef MAP_RENDER_H
#define MAP_RENDER_H
#include "graphics/view.h"
void map_renderer_init(void);
void map_renderer_fini(void);
void map_render(OthoCamera* view);
#endif //MAP_RENDER_H
