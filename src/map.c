#include "map.h"
#include "constances.h"
#include <string.h>

#define ANIM_CELL_CNT_MAX 200
#define TRAP_TILE 10
#define RED_FOUNDTAIN_TOP 38
#define RED_FOUNDTAIN_BASIN 32
#define BLUE_FOUNTAIN_TOP 35
#define BLUE_FOUNDTAIN_BASIN 29

typedef struct AnimatedTile
{
  tile_t tiles[4];          /* point to static tile     */
  int    duration;          /* frame duration           */
  int    count;             /* tile count               */
  int    currentFrameIndex; /* current key frame index  */
  tile_t currentFrame;      /* current key frame        */
} AnimatedTile;

typedef struct AnimatedCell
{
  int                 row;
  int                 col;
  int                 layer;
  const AnimatedTile* animatedTile;
} AnimatedCell;

enum
{
  FLOOR_LAYER,
  WALL_LAYER
};

extern SDL_Rect      gViewport;
extern SDL_Renderer* gRenderer;

tile_t gFloor[MAP_MAX_TILES];
tile_t gWall[MAP_MAX_TILES];
float  gWallZ[MAP_MAX_TILES];
s32    gMapHeight;
s32    gMapWidth;

static s32          sTicks;
static AnimatedCell sAnimatedCells[ANIM_CELL_CNT_MAX];
static int          sAnimatedCellCnt;

enum
{
  ANIM_TILE_TRAP,
  ANIM_TILE_RED_FOUNTAIN_TOP,
  ANIM_TILE_RED_FOUNTAIN_BASIN,
  ANIM_TILE_BLUE_FOUNTAIN_TOP,
  ANIM_TILE_BLUE_FOUNTAIN_BASIN,
  NUM_ANIMATED_TILES,
};

static AnimatedTile sAnimatedTiles[NUM_ANIMATED_TILES] = {
  [ANIM_TILE_TRAP]                = { .tiles = { 10, 11, 12, 13 }, .count = 4, .duration = 40, },
  [ANIM_TILE_BLUE_FOUNTAIN_TOP]   = { .tiles = { 35, 36, 37 }, .count = 3, .duration = 20, },
  [ANIM_TILE_BLUE_FOUNTAIN_BASIN] = { .tiles = { 29, 30, 31 }, .count = 3, .duration = 20, },
  [ANIM_TILE_RED_FOUNTAIN_TOP]    = { .tiles = { 38, 39, 40 }, .count = 3, .duration = 20, },
  [ANIM_TILE_RED_FOUNTAIN_BASIN]  = { .tiles = { 32, 33, 34 }, .count = 3, .duration = 20, },
};

void
map_create_animated_cell(int layer, int row, int col, int animTileId)
{
  if (sAnimatedCellCnt < ANIM_CELL_CNT_MAX)
  {
    AnimatedCell* cell = &sAnimatedCells[sAnimatedCellCnt++];
    cell->row          = row;
    cell->col          = col;
    cell->animatedTile = &sAnimatedTiles[animTileId];
    cell->layer        = layer;
  }
}

static void
update_animated_tiles()
{
  AnimatedTile* tile;
  ++sTicks;
  for (int i = 0; i < NUM_ANIMATED_TILES; ++i)
  {
    tile                    = &sAnimatedTiles[i];
    tile->currentFrameIndex = (sTicks / tile->duration) % tile->count;
    tile->currentFrame      = tile->tiles[tile->currentFrameIndex];
  }
}

static int
to_animated_tile_id(s32 tile)
{
  int animTileId = -1;
  switch (tile)
  {
  case TRAP_TILE:
    animTileId = ANIM_TILE_TRAP;
    break;
  case BLUE_FOUNTAIN_TOP:
    animTileId = ANIM_TILE_BLUE_FOUNTAIN_TOP;
    break;
  case BLUE_FOUNDTAIN_BASIN:
    animTileId = ANIM_TILE_BLUE_FOUNTAIN_BASIN;
    break;
  case RED_FOUNDTAIN_TOP:
    animTileId = ANIM_TILE_BLUE_FOUNTAIN_TOP;
    break;
  case RED_FOUNDTAIN_BASIN:
    animTileId = ANIM_TILE_BLUE_FOUNTAIN_BASIN;
    break;
  }
  return animTileId;
}

static void
create_animated_cells()
{
  int tileCnt, animTileId;
  int i;
  int col;
  int row;

  tileCnt = gMapHeight * gMapWidth;

  for (i = 0; i < tileCnt; ++i)
  {
    animTileId = to_animated_tile_id(gWall[i]);
    if (animTileId != -1)
    {
      col = i % gMapWidth;
      row = i / gMapWidth;
      map_create_animated_cell(WALL_LAYER, row, col, animTileId);
    }
  }

  for (i = 0; i < tileCnt; ++i)
  {
    animTileId = to_animated_tile_id(gFloor[i]);
    if (animTileId != -1)
    {
      col = i % gMapWidth;
      row = i / gMapWidth;
      map_create_animated_cell(FLOOR_LAYER, row, col, animTileId);
    }
  }
}

void
map_tick()
{
  update_animated_tiles();
  const AnimatedCell* cell;
  for (int i = 0; i < sAnimatedCellCnt; ++i)
  {
    cell = &sAnimatedCells[i];
    if (cell->layer == WALL_LAYER)
      setwall(cell->col, cell->row, cell->animatedTile->currentFrame);
    else
      setfloor(cell->col, cell->row, cell->animatedTile->currentFrame);
  }
}

BOOL
is_roof(tile_t tile)
{
#define _(x) [x]      = UZU_TRUE
  static BOOL lut[65] = { _(19), _(20), _(25), _(26), _(45), _(46),
                          _(49), _(50), _(58), _(59), _(60), _(61),
                          _(62), _(63), _(64), _(40), _(18) };
#undef _
  return lut[tile - 1];
}

static void setwallz(int x, int y, float z)
{
    gWallZ[x + y * gMapWidth] = z;
}

static void
init_wall_tile_draw_order(void)
{
  tile_t tile;
  for (int y = gMapHeight - 1; y >= 0; --y)
    for (int x = 0; x < gMapWidth; ++x)
    {
      tile = x < gMapWidth && y < gMapHeight ?
                  getwall(x, y) : 0;
      if (tile != 0 && is_roof(tile) && y < gMapHeight - 1)
        setwallz(x, y, getwallz(x, y + 1));
      else
        setwallz(x, y, -((y + 1) * TILE_SIZE) / 100.f);
    }
}

void
map_init(const tile_t* floor, const tile_t* wall, int width, int height)
{
  int count;

  gMapHeight       = height;
  gMapWidth        = width;
  sAnimatedCellCnt = 0;
  count            = width * height;
  SDL_memcpy(gFloor, floor, count * sizeof(tile_t));
  SDL_memcpy(gWall, wall, count * sizeof(tile_t));
  create_animated_cells();
  init_wall_tile_draw_order();
}

void
map_clear()
{
  sAnimatedCellCnt = 0;
  gMapWidth        = 0;
  gMapHeight       = 0;
  SDL_memset(gFloor, 0, sizeof(gFloor));
  SDL_memset(gWall, 0, sizeof(gWall));
}
