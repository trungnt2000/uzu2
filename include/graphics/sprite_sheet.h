// ngotrung Wed 18 Aug 2021 05:36:09 PM +07
#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H
#include "graphics/asset_table.h"
#include "graphics/types.h"
typedef struct SpriteSheet
{
  AssetTable assetTable;
  Texture    texture;
} SpriteSheet;

int  sprite_sheet_load(SpriteSheet* sheet, const char* file);
void sprite_sheet_destroy(SpriteSheet* sheet);

const TextureRegion* sprite_sheet_get(SpriteSheet* sheet, const char* name);
#endif // SPRITE_SHEET_H
