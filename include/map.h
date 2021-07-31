#ifndef MAP_H
#define MAP_H

#include "cglm/cglm.h"
#include "toolbox.h"

#define MAP_MAX_ROW 300
#define MAP_MAX_COL 300
#define MAP_MAX_TILES (MAP_MAX_ROW * MAP_MAX_COL)

typedef int tile_t;

extern tile_t gFloor[];
extern tile_t gWall[];
extern float  gWallZ[];
extern s32    gMapHeight;
extern s32    gMapWidth;
extern vec2   gMapPos;
extern float  gMapRot;
extern vec2   gMapScl;

// clang-format off
static inline void
setwall(int x, int y, tile_t tile)
{ gWall[x + y * gMapWidth] = tile; }

static inline void
setfloor(int x, int y, tile_t tile)
{ gFloor[x + y * gMapWidth] = tile; }

static inline tile_t
getwall(int x, int y)
{ return gWall[x + y * gMapWidth]; }

static inline tile_t
getfloor(int x, int y)
{ return gFloor[x + y * gMapWidth]; }

static inline float
getwallz(int x, int y)
{ return gWallZ[x + y * gMapWidth]; }
// clang-format on

void map_tick(void);
void map_init(const tile_t* floor, const tile_t* wall, int width, int height);
void map_clear(void);

#endif // MAP_H
