#ifndef MAP_H
#define MAP_H

#include "cglm/cglm.h"
#include "toolbox.h"

#define MAP_MAX_ROW 300
#define MAP_MAX_COL 300
#define MAP_MAX_TILES (MAP_MAX_ROW * MAP_MAX_COL)

typedef int tile_t;

extern tile_t g_floor[];
extern tile_t g_wall[];
extern float  g_wall_draw_order[];
extern s32    g_map_height;
extern s32    g_map_width;
extern vec2   gMapPos;
extern float  gMapRot;
extern vec2   gMapScl;

// clang-format off
static inline void
setwall(int x, int y, tile_t tile)
{ g_wall[x + y * g_map_width] = tile; }

static inline void
setfloor(int x, int y, tile_t tile)
{ g_floor[x + y * g_map_width] = tile; }

static inline tile_t
getwall(int x, int y)
{ return g_wall[x + y * g_map_width]; }

static inline tile_t
getfloor(int x, int y)
{ return g_floor[x + y * g_map_width]; }

static inline float
getwallz(int x, int y)
{ return g_wall_draw_order[x + y * g_map_width]; }
// clang-format on

void map_tick(void);
void map_init(const tile_t* floor, const tile_t* wall, int width, int height);
void map_clear(void);

#endif // MAP_H
