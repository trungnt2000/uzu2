#include "graphics/sprite_sheet.h"
#include "json_helper.h"


int
sprite_sheet_load(SpriteSheet* sheet, const char* file)
{
  json_object*   sheet_data;
  json_object*   meta_data;
  json_object*   frames_data;
  json_object*   tmp;
  json_object*   tmp2;
  json_object*   frame_data;
  const char*    texture_file;
  size_t         sprite_count;
  Sprite* sprite;
  const char*    spriteName;
  IntRect        rect;

  sheet_data = json_from_file(file);
  if (sheet_data == NULL)
  {
    UZU_ERROR("Failed to load sprite sheet mete file\n");
    return -1;
  }

  meta_data   = json_object_object_get(sheet_data, "meta");
  frames_data = json_object_object_get(sheet_data, "frames");

  if (meta_data == NULL || frames_data == NULL)
  {
    UZU_ERROR("Invalid sprite sheet file\n");
    return -1;
  }

  tmp = json_object_object_get(meta_data, "image");
  if (tmp == NULL)
  {
    UZU_ERROR("Invalid sprite sheet file\n");
    return -1;
  }

  texture_file = json_object_get_string(tmp);
  if (texture_load(&sheet->texture, texture_file) == -1)
  {
    UZU_ERROR("Failed to load texture file\n");
    return -1;
  }
  asset_table_init(&sheet->asset_table, SDL_free);
  sprite_count = json_object_array_length(frames_data);
  for (size_t i = 0; i < sprite_count; ++i)
  {
    frame_data = json_object_array_get_idx(frames_data, i);
    sprite    = SDL_malloc(sizeof *sprite);

    tmp    = json_object_object_get(frame_data, "frame");
    tmp2   = json_object_object_get(tmp, "x");
    rect.x = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "y");
    rect.y = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "w");
    rect.w = json_object_get_int(tmp2);

    tmp2   = json_object_object_get(tmp, "h");
    rect.h = json_object_get_int(tmp2);

    tmp        = json_object_object_get(frame_data, "filename");
    spriteName = json_object_get_string(tmp);

    sprite_init(sprite, &sheet->texture, &rect);
    asset_table_insert(&sheet->asset_table, spriteName, sprite);
  }

  json_object_put(sheet_data);

  return 0;
}

void
sprite_sheet_destroy(SpriteSheet* sheet)
{
  asset_table_destroy(&sheet->asset_table);
  texture_destroy(&sheet->texture);
}

const Sprite*
sprite_sheet_get(const SpriteSheet* sheet, const char* name)
{
  return (const Sprite*)asset_table_lookup(&sheet->asset_table, name);
}
