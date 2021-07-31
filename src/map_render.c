#include "SDL.h"
#include "constances.h"
#include "graphics.h"
#include "graphics/view.h"
#include "input.h"
#include "map.h"

/* how many float components in one vertex                   */
/* each vertex contains two floats for position,             */
/* two floats for texture coordinate and final one for depth */
#define NUM_VERT_COMPS 5

/* how many byte is one tile vertex take in byte */
#define TILE_VERT_SIZ (sizeof(float) * NUM_VERT_COMPS)

/* maximum amount of tile can be batched together in single batch */
#define MAX_TILES (1024)

#define FLOOR_Z (1.f)

/* struct store four texture coords of a tile */
typedef struct Tile
{
  float u1;
  float v1;
  float u2;
  float v2;
} Tile;

/* shader for rendering tilemap */
static GLuint sShader;

/* where view-projecttion matrix is located inside our shader */
static int sViewProjectMatrixLocation;

/* our vertex array object */
static GLuint sVao;

/* our vertex buffer object which store two dimensional coord and text coord*/
static GLuint sVbo;

/* our element buffer object */
static GLuint sEbo;

/* our vertex buffer in cpu side: each vertex take 5 floats and each tile take
 * 4 vertices */
static float sVertBuf[MAX_TILES * NUM_VERT_COMPS * 4];

/* how many tiles are store in current bacth */
static int sCnt;

static Tile sTileset[512];
static int  sTilesetSiz;

/* texture use for tileset */
static Texture sTilesetTex;

static int sDrawCallCnt;
static int sVertCnt;

/****************************************************/
/*  hepler functions */
static int  load_resources(void);
static void init_vao(void);
static int  init_tileset(void);
static void prepare(void);
static void flush(void);

void
map_render_init(void)
{
  load_resources();
  init_tileset();
  init_vao();
}

void
map_render_fini(void)
{
  glDeleteProgram(sShader);
  glDeleteBuffers(1, &sVbo);
  glDeleteBuffers(1, &sEbo);
  glDeleteVertexArrays(1, &sVao);
  texture_free(&sTilesetTex);
}

void
map_render(void)
{
  Tile   tile;
  tile_t tileId;
  int    idx = sCnt * NUM_VERT_COMPS * 4;
  float  x1, x2, y1, y2;
  float  wallz;
  int    x, y, i, j;
  int    firstCol, firstRow, lastCol, lastRow;

  prepare();

  firstCol = (int)view_left() / TILE_SIZE;
  firstRow = (int)view_top() / TILE_SIZE;

  lastCol = (int)view_right() / TILE_SIZE;
  lastRow = (int)view_bot() / TILE_SIZE;

  firstCol = max(0, firstCol);
  firstRow = max(0, firstRow);

  lastCol = min(lastCol, gMapWidth - 1);
  lastRow = min(lastRow, gMapHeight - 1);

  /* draw wall */
  for (i = firstRow, y = firstRow * TILE_SIZE; i <= lastRow;
       ++i, y += TILE_SIZE)
  {
    for (j = firstCol, x = firstCol * TILE_SIZE; j <= lastCol;
         ++j, x += TILE_SIZE)
    {
      tileId = getwall(j, i);
      if (tileId == 0)
        continue;

      if (sCnt == MAX_TILES)
      {
        flush();
        idx = 0;
      }

      x1    = x;
      x2    = x + TILE_SIZE;
      y1    = y;
      y2    = y + TILE_SIZE;
      wallz = getwallz(x, y);
      tile  = sTileset[tileId - 1];

      /* top-left */
      sVertBuf[idx++] = x1;
      sVertBuf[idx++] = y1;
      sVertBuf[idx++] = wallz;
      sVertBuf[idx++] = tile.u1;
      sVertBuf[idx++] = tile.v1;

      /* top-right */
      sVertBuf[idx++] = x2;
      sVertBuf[idx++] = y1;
      sVertBuf[idx++] = wallz;
      sVertBuf[idx++] = tile.u2;
      sVertBuf[idx++] = tile.v1;

      /* bottom-right */
      sVertBuf[idx++] = x2;
      sVertBuf[idx++] = y2;
      sVertBuf[idx++] = wallz;
      sVertBuf[idx++] = tile.u2;
      sVertBuf[idx++] = tile.v2;

      /* bottom-left */
      sVertBuf[idx++] = x1;
      sVertBuf[idx++] = y2;
      sVertBuf[idx++] = wallz;
      sVertBuf[idx++] = tile.u1;
      sVertBuf[idx++] = tile.v2;

      sCnt++;
    }
  }
  /* draw floor */
  for (i = firstRow, y = firstRow * TILE_SIZE; i <= lastRow;
       ++i, y += TILE_SIZE)
  {
    for (j = firstCol, x = firstCol * TILE_SIZE; j <= lastCol;
         ++j, x += TILE_SIZE)
    {
      tileId = getfloor(j, i);
      if (tileId == 0)
        continue;

      if (sCnt == MAX_TILES)
      {
        flush();
        idx = 0;
      }

      x1   = x;
      x2   = x + TILE_SIZE;
      y1   = y;
      y2   = y + TILE_SIZE;
      tile = sTileset[tileId - 1];

      /* top-left */
      sVertBuf[idx++] = x1;
      sVertBuf[idx++] = y1;
      sVertBuf[idx++] = FLOOR_Z;
      sVertBuf[idx++] = tile.u1;
      sVertBuf[idx++] = tile.v1;

      /* top-right */
      sVertBuf[idx++] = x2;
      sVertBuf[idx++] = y1;
      sVertBuf[idx++] = FLOOR_Z;
      sVertBuf[idx++] = tile.u2;
      sVertBuf[idx++] = tile.v1;

      /* bottom-right */
      sVertBuf[idx++] = x2;
      sVertBuf[idx++] = y2;
      sVertBuf[idx++] = FLOOR_Z;
      sVertBuf[idx++] = tile.u2;
      sVertBuf[idx++] = tile.v2;

      /* bottom-left */
      sVertBuf[idx++] = x1;
      sVertBuf[idx++] = y2;
      sVertBuf[idx++] = FLOOR_Z;
      sVertBuf[idx++] = tile.u1;
      sVertBuf[idx++] = tile.v2;
      ++sCnt;
    }
  }
  flush();
}

static void
flush()
{
  // update our vertex buffer
  mat4 viewProjectMatrix;
  u32  updtSiz   = sCnt * TILE_VERT_SIZ * 4;
  int  indiceCnt = sCnt * 6;
  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, updtSiz, sVertBuf);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  view_combined(viewProjectMatrix);
  glUseProgram(sShader);
  glUniformMatrix4fv(sViewProjectMatrixLocation,
                     1,
                     GL_FALSE,
                     (float*)viewProjectMatrix);
  glBindVertexArray(sVao);
  texture_bind(&sTilesetTex);
  glEnable(GL_DEPTH_TEST);

  // draw all tiles
  glBindVertexArray(sVao);
  glDrawElements(GL_TRIANGLES, indiceCnt, GL_UNSIGNED_SHORT, (void*)0);
  glDisable(GL_DEPTH_TEST);

  sCnt = 0;
  sDrawCallCnt++;
  sVertCnt += indiceCnt;
}

static int
load_resources(void)
{
  /* load shaders */
  if (create_shader("res/shader/tile.vert", "res/shader/tile.frag", &sShader) !=
      0)
  {
    ERROR("fail to load map shader\n");
    return -1;
  }
  sViewProjectMatrixLocation = glGetUniformLocation(sShader, "u_vpMat");

  /* load textures */
  if (texture_load(&sTilesetTex, "res/titleset.png") != 0)
  {
    ERROR("fail to load titleset texture\n");
    return -1;
  }
  return 0;
}

static void
init_vao(void)
{
  size_t       attrOffset;
  u16*         indices = NULL;
  const size_t eboSiz  = sizeof(u16) * MAX_TILES * 6;

  indices = SDL_malloc(eboSiz);
  for (u16 i = 0; i < MAX_TILES; ++i)
  {
    indices[(i * 6) + 0] = (i * 4) + 0;
    indices[(i * 6) + 1] = (i * 4) + 1;
    indices[(i * 6) + 2] = (i * 4) + 3;
    indices[(i * 6) + 3] = (i * 4) + 1;
    indices[(i * 6) + 4] = (i * 4) + 2;
    indices[(i * 6) + 5] = (i * 4) + 3;
  }

  glGenVertexArrays(1, &sVao);
  glGenBuffers(1, &sVbo);
  glGenBuffers(1, &sEbo);

  glBindVertexArray(sVao);

  glBindBuffer(GL_ARRAY_BUFFER, sVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sVertBuf), 0, GL_DYNAMIC_DRAW);

  // struct {vec3 pos; vec2 uv;}
  attrOffset = 0;
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        TILE_VERT_SIZ,
                        (void*)attrOffset);
  glEnableVertexAttribArray(0);
  attrOffset += sizeof(float) * 3;

  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        TILE_VERT_SIZ,
                        (void*)attrOffset);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSiz, indices, GL_STATIC_DRAW);

  glBindVertexArray(0);

  SDL_free(indices);
}

static void
prepare(void)
{
  sCnt         = 0;
  sDrawCallCnt = 0;
  sVertCnt     = 0;
}

static int
init_tileset(void)
{
  int   colCnt    = sTilesetTex.width / TILE_SIZE;
  int   rowCnt    = sTilesetTex.height / TILE_SIZE;
  int   idx       = 0;
  float texWidth  = sTilesetTex.width;
  float texHeight = sTilesetTex.height;

  for (int i = 0; i < rowCnt; ++i)
  {
    for (int j = 0; j < colCnt; ++j)
    {
      sTileset[idx].u1 = (j * TILE_SIZE) / texWidth;
      sTileset[idx].u2 = ((j + 1) * TILE_SIZE) / texWidth;
      sTileset[idx].v1 = (i * TILE_SIZE) / texHeight;
      sTileset[idx].v2 = ((i + 1) * TILE_SIZE) / texHeight;
      ++idx;
    }
  }
  sTilesetSiz = idx;
  return 0;
}
