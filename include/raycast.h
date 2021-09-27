// ngotrung Mon 13 Sep 2021 11:23:14 PM +07
#ifndef RAYCAST_H
#define RAYCAST_H
#include <stdbool.h>

struct Gird
{
    int* tiles;
    int  width;
    int  height;
    int  tile_width;
    int  tile_height;
};

struct RaycastInput
{
    float pos_x;
    float pos_y;
    float dir_x;
    float dir_y;
    float length;
    bool (*callback)(void*, int tile_x, int tile_y);
    void* ctx;
};

void raycast(const struct Gird* gird, const struct RaycastInput* inp);

void astar(const struct Gird* gird,
           int                start_x,
           int                start_y,
           int                end_x,
           int                end_y,
           int*               path_ret,
           int                max,
           int*               count_ret);

#endif // RAYCAST_H
