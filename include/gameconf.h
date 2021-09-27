// ngotrung Tue 07 Sep 2021 09:57:31 AM +07
#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdbool.h>

typedef struct GameConf
{
  int  width;
  int  height;
  bool vsync;
} GameConf;

#define GAME_CONF_INIT_DFT                                                                                       \
  {                                                                                                              \
    .width = 640, .height = 480                                                                                  \
  }

#endif // GAME_CONFIG_H
