#include "cglm/cglm.h"
#include "toolbox/quad_tree.h"

struct Gird
{
    int* tiles;
};

struct TileCollisionInfo
{
    int        tile_x;
    int        tile_y;
    struct Box tile_bounds;
    struct Box entity_bounds;
};

void resolve_collision(struct TileCollisionInfo collision_info, vec2 position_ret, vec2 velocity_ret);
