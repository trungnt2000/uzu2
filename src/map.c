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
  tile_t tiles[4];            /* point to static tile     */
  int    duration;            /* frame duration           */
  int    count;               /* tile count               */
  int    current_frame_index; /* current key frame index  */
  tile_t current_frame;       /* current key frame        */
} AnimatedTile;

typedef struct AnimatedCell
{
  int                 row;
  int                 col;
  int                 layer;
  const AnimatedTile* animated_tile;
} AnimatedCell;

enum
{
  FLOOR_LAYER,
  WALL_LAYER
};

tile_t g_floor[MAP_MAX_TILES];
tile_t g_wall[MAP_MAX_TILES];
float  g_wall_draw_order[MAP_MAX_TILES];
s32    g_map_height;
s32    g_map_width;

static s32          s_ticks;
static AnimatedCell s_animated_cells[ANIM_CELL_CNT_MAX];
static int          s_animated_cell_cnt;

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
  if (s_animated_cell_cnt < ANIM_CELL_CNT_MAX)
  {
    AnimatedCell* cell  = &s_animated_cells[s_animated_cell_cnt++];
    cell->row           = row;
    cell->col           = col;
    cell->animated_tile = &sAnimatedTiles[animTileId];
    cell->layer         = layer;
  }
}

static void
update_animated_tiles()
{
  AnimatedTile* tile;
  ++s_ticks;
  for (int i = 0; i < NUM_ANIMATED_TILES; ++i)
  {
    tile                      = &sAnimatedTiles[i];
    tile->current_frame_index = (s_ticks / tile->duration) % tile->count;
    tile->current_frame       = tile->tiles[tile->current_frame_index];
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

  tileCnt = g_map_height * g_map_width;

  for (i = 0; i < tileCnt; ++i)
  {
    animTileId = to_animated_tile_id(g_wall[i]);
    if (animTileId != -1)
    {
      col = i % g_map_width;
      row = i / g_map_width;
      map_create_animated_cell(WALL_LAYER, row, col, animTileId);
    }
  }

  for (i = 0; i < tileCnt; ++i)
  {
    animTileId = to_animated_tile_id(g_floor[i]);
    if (animTileId != -1)
    {
      col = i % g_map_width;
      row = i / g_map_width;
      map_create_animated_cell(FLOOR_LAYER, row, col, animTileId);
    }
  }
}

void
map_tick()
{
  update_animated_tiles();
  const AnimatedCell* cell;
  for (int i = 0; i < s_animated_cell_cnt; ++i)
  {
    cell = &s_animated_cells[i];
    if (cell->layer == WALL_LAYER)
      setwall(cell->col, cell->row, cell->animated_tile->current_frame);
    else
      setfloor(cell->col, cell->row, cell->animated_tile->current_frame);
  }
}

bool
is_roof(tile_t tile)
{
#define _(x) [x]      = true
  static bool lut[65] = { _(19), _(20), _(25), _(26), _(45), _(46), _(49), _(50), _(58),
                          _(59), _(60), _(61), _(62), _(63), _(64), _(40), _(18) };
#undef _
  return lut[tile - 1];
}

static void
setwallz(int x, int y, float z)
{
  g_wall_draw_order[x + y * g_map_width] = z;
}

static void
init_wall_tile_draw_order(void)
{
  tile_t tile;
  for (int y = g_map_height - 1; y >= 0; --y)
    for (int x = 0; x < g_map_width; ++x)
    {
      tile = x < g_map_width && y < g_map_height ? getwall(x, y) : 0;
      if (tile != 0 && is_roof(tile) && y < g_map_height - 1)
        setwallz(x, y, getwallz(x, y + 1));
      else
        setwallz(x, y, -((y + 1) * TILE_SIZE) / 100.f);
    }
}

void
map_init(const tile_t* floor, const tile_t* wall, int width, int height)
{
  int count;

  g_map_height        = height;
  g_map_width         = width;
  s_animated_cell_cnt = 0;
  count               = width * height;
  SDL_memcpy(g_floor, floor, count * sizeof(tile_t));
  SDL_memcpy(g_wall, wall, count * sizeof(tile_t));
  create_animated_cells();
  init_wall_tile_draw_order();
}

void
map_clear()
{
  s_animated_cell_cnt = 0;
  g_map_width         = 0;
  g_map_height        = 0;
  SDL_memset(g_floor, 0, sizeof(g_floor));
  SDL_memset(g_wall, 0, sizeof(g_wall));
}
