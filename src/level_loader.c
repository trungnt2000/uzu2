#include "level_loader.h"
#include "json_helper.h"
#include "map.h"
#include <json-c/json.h>

#define LEVEL_DATA_DIR "res/level/"

static void
level_name_to_file_name(const char* levelName, char* dest)
{
    strcpy(dest, LEVEL_DATA_DIR);
    strcat(dest, levelName);
    strcat(dest, ".json");
}

/*
static int item_id_from_string(const char* name)
{
  static const char* lut[NUM_ITEM_TYPES] = {
    [ITEM_TYPE_RED_FLASK]        = "RedFlask",
    [ITEM_TYPE_BIG_RED_FLASK]    = "BigRedFlask",
    [ITEM_TYPE_BLUE_FLASK]       = "BlueFlask",
    [ITEM_TYPE_SCROLL_ICE_ARROW] = "IceArrowScroll",
    [ITEM_TYPE_SCROLL_FIRE_BALL] = "FireBallScroll",
    [ITEM_TYPE_KEY_1_1]          = "Key_1_1",
    [ITEM_TYPE_STAFF]            = "RedStaff",
  };
  for (int i = 0; i < NUM_ITEM_TYPES; ++i)
    if (lut[i] && SDL_strcmp(lut[i], name) == 0)
      return i;
  return -1;
}

static int parse_tilelayer(const json_object* jsoTileLayer, tile_t* data);
static int parse_objectgroup(const json_object* jobjectgroup);

static void parse_imp(ecs_Registry* registry, const EntityProperties* params);
static void parse_wogol(ecs_Registry* registry, const EntityProperties* params);
static void parse_huge_demon(ecs_Registry*           registry,
                             const EntityProperties* params);
static void parse_chest(ecs_Registry* registry, const EntityProperties* params);
static void parse_ladder(ecs_Registry*           registry,
                         const EntityProperties* params);
static void parse_chort(ecs_Registry* registry, const EntityProperties* params);
static void parse_door(ecs_Registry* registry, const EntityProperties* params);

static void parse_item(Item* item, json_object* json)
{
  ASSERT(json != NULL);
  const char* itemName;
  int         numItems;

  itemName      = json_get_string(json, "type");
  numItems      = json_get_int(json, "quality");
  item->type    = item_id_from_string(itemName);
  item->quality = numItems;
}

static void parse_item_list(Item items[5], u16* num_items, const char* input)
{
  json_object* json = NULL;

  int n;
  if (input == NULL)
    return;

  json = json_tokener_parse(input);

  n = json_object_array_length(json);

  n = min(CHEST_MAX_ITEMS, n);
  for (int i = 0; i < n; ++i)
  {
    parse_item(&items[i], json_object_array_get_idx(json, i));
  }

  *num_items = n;
  json_object_put(json);
}
*/
static json_object*
find_layer(json_object* layers_data, const char* layer_name)
{
    size_t       layer_count = json_object_array_length(layers_data);
    json_object* layer_data;
    for (size_t i = 0; i < layer_count; ++i)
    {
        layer_data = json_object_array_get_idx(layers_data, i);
        if (!SDL_strcmp(layer_name, json_get_string(layer_data, "name")))
            return layer_data;
    }
    return NULL;
}

static int
parse_tilelayer(const json_object* tile_layer_data, tile_t* layer)
{
    size_t             tile_count;
    const json_object* tiles_data;

    tiles_data = json_object_object_get(tile_layer_data, "data");
    tile_count = json_object_array_length(tiles_data);

    for (size_t i = 0; i < tile_count; ++i)
        layer[i] = json_array_get_int(tiles_data, i);

    return 0;
}

static int
parse(const json_object* map_data)
{
    json_object *layers_data, *wall_layer_data, *floor_layer_data;
    int          w, h;
    tile_t*      floor;
    tile_t*      wall;
    int          status = 0;

    layers_data = json_object_object_get(map_data, "layers");
    w           = json_get_int(map_data, "width");
    h           = json_get_int(map_data, "height");

    floor = SDL_malloc(w * h * sizeof(tile_t));
    wall  = SDL_malloc(w * h * sizeof(tile_t));

    wall_layer_data  = find_layer(layers_data, "wall");
    floor_layer_data = find_layer(layers_data, "floor");

    if (floor_layer_data == NULL || SDL_strcmp(json_get_string(floor_layer_data, "type"), "tilelayer"))
    {
        UZU_ERROR("map data does not contain floor layer or it isn't a tilelayer");
        status = -1;
        goto cleanup;
    }

    if (wall_layer_data == NULL || SDL_strcmp(json_get_string(wall_layer_data, "type"), "tilelayer"))
    {
        UZU_ERROR("map data does not contain wall layer or it isn't a tilelayer");
        status = -1;
        goto cleanup;
    }
    parse_tilelayer(floor_layer_data, floor);
    parse_tilelayer(wall_layer_data, wall);
    map_init(floor, wall, w, h);

cleanup:
    SDL_free(floor);
    SDL_free(wall);

    return status;
}

int
load_level(const char* level_name)
{
    char                filename[255] = "";
    struct json_object* map_data      = NULL;
    int                 status        = 0;
    level_name_to_file_name(level_name, filename);

    if ((map_data = json_from_file(filename)) == NULL)
    {
        status = -1;
        goto cleanup;
    }
    if (parse(map_data) != 0)
    {
        status = -1;
        goto cleanup;
    }
cleanup:
    json_object_put(map_data);
    return status;
}
