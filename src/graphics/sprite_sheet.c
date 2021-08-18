#include "graphics/sprite_sheet.h"
#include "json_helper.h"


int
sprite_sheet_load(SpriteSheet* sheet, const char* file)
{
  json_object*   sheetData;
  json_object*   metaData;
  json_object*   framesData;
  json_object*   tmp;
  json_object*   tmp2;
  json_object*   frameData;
  const char*    textureFile;
  size_t         spriteCount;
  TextureRegion* sprite;
  const char*    spriteName;
  IntRect        rect;

  sheetData = json_from_file(file);
  if (sheetData == NULL)
  {
    UZU_ERROR("Failed to load sprite sheet mete file\n");
    return -1;
  }

  metaData   = json_object_object_get(sheetData, "meta");
  framesData = json_object_object_get(sheetData, "frames");

  if (metaData == NULL || framesData == NULL)
  {
    UZU_ERROR("Invalid sprite sheet file\n");
    return -1;
  }

  tmp = json_object_object_get(metaData, "image");
  if (tmp == NULL)
  {
    UZU_ERROR("Invalid sprite sheet file\n");
    return -1;
  }

  textureFile = json_object_get_string(tmp);
  if (texture_load(&sheet->texture, textureFile) == -1)
  {
    UZU_ERROR("Failed to load texture file\n");
    return -1;
  }
  asset_table_init(&sheet->assetTable, SDL_free);
  spriteCount = json_object_array_length(framesData);
  for (size_t i = 0; i < spriteCount; ++i)
  {
    frameData = json_object_array_get_idx(framesData, i);
    sprite    = SDL_malloc(sizeof *sprite);

    tmp    = json_object_object_get(frameData, "frame");
    tmp2   = json_object_object_get(tmp, "x");
    rect.x = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "y");
    rect.y = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "w");
    rect.w = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "h");
    rect.h = json_object_get_int(tmp2);

    tmp        = json_object_object_get(frameData, "filename");
    spriteName = json_object_get_string(tmp);

    texture_region_init(sprite, &sheet->texture, &rect);
    asset_table_insert(&sheet->assetTable, spriteName, sprite);
  }

  json_object_put(sheetData);

  return 0;
}

void
sprite_sheet_destroy(SpriteSheet* sheet)
{
  asset_table_destroy(&sheet->assetTable);
  texture_destroy(&sheet->texture);
}

const TextureRegion*
sprite_sheet_get(SpriteSheet* sheet, const char* name)
{
  return (const TextureRegion*)asset_table_lookup(&sheet->assetTable, name);
}
