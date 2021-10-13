#include "graphics/sprite_sheet.h"
#include "json_helper.h"

static int
sprite_sheet_parse_json(struct SpriteSheet* sheet, json_object* sheet_data)
{
    json_object*   meta_data;
    json_object*   frames_data;
    json_object*   tmp;
    json_object*   tmp2;
    json_object*   frame_data;
    const char*    texture_file;
    size_t         sprite_count;
    struct Sprite* sprite;
    const char*    sprite_name;
    IntRect        rect;
    meta_data   = json_object_object_get(sheet_data, "meta");
    frames_data = json_object_object_get(sheet_data, "frames");

    if (meta_data == NULL || frames_data == NULL)
        LEAVE_ERROR(-1, "Invalid sprite sheet file\n");

    tmp = json_object_object_get(meta_data, "image");
    if (tmp == NULL)
        LEAVE_ERROR(-1, "Invalid sprite sheet file\n");

    texture_file = json_object_get_string(tmp);
    if (texture_load(&sheet->texture, texture_file) == -1)
        LEAVE_ERROR(-1, "Failed to load texture file\n");

    asset_table_init(&sheet->asset_table, SDL_free);
    sprite_count = json_object_array_length(frames_data);
    for (size_t i = 0; i < sprite_count; ++i)
    {
        frame_data = json_object_array_get_idx(frames_data, i);
        sprite     = SDL_malloc(sizeof *sprite);

        tmp    = json_object_object_get(frame_data, "frame");
        tmp2   = json_object_object_get(tmp, "x");
        rect.x = json_object_get_int(tmp2);

        tmp2   = json_object_object_get(tmp, "y");
        rect.y = json_object_get_int(tmp2);

        tmp2   = json_object_object_get(tmp, "w");
        rect.w = json_object_get_int(tmp2);

        tmp2   = json_object_object_get(tmp, "h");
        rect.h = json_object_get_int(tmp2);

        tmp         = json_object_object_get(frame_data, "filename");
        sprite_name = json_object_get_string(tmp);

        sprite_init(sprite, &sheet->texture, &rect);
        asset_table_insert(&sheet->asset_table, sprite_name, sprite);
    }

    return 0;
}

int
sprite_sheet_load(struct SpriteSheet* sheet, const char* file)
{
    json_object* sheet_data;

    sheet_data = json_from_file(file);
    if (sheet_data == NULL)
        LEAVE_ERROR(-1, "Failed to load sprite sheet mete file\n");

    if (sprite_sheet_parse_json(sheet, sheet_data) != 0)
        LEAVE_ERROR(-1, "parse sheet data failed\n");

    json_object_put(sheet_data);
    return 0;
}

void
sprite_sheet_destroy(struct SpriteSheet* sheet)
{
    asset_table_destroy(&sheet->asset_table);
    texture_destroy(&sheet->texture);
}

const Sprite*
sprite_sheet_get(const struct SpriteSheet* sheet, const char* name)
{
    return (const Sprite*)asset_table_lookup(&sheet->asset_table, name);
}
