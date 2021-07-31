#include "level_loader.h"
#include "json-c/json.h"
#include "json_helper.h"
#include "map.h"

#define LEVEL_DATA_DIR "res/level/"

/*
extern ecs_Registry* gRegistry;

typedef struct EntityProperties
{
  Vec2         position;
  Vec2         size;
  const char*  name;
  json_object* properties;
  u16          id;
} EntityProperties;
*/
static void level_name_to_file_name(const char* levelName, char* dest)
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
static json_object* find_layer(json_object* layers, const char* layerName)
{
  int          nlayer = json_object_array_length(layers);
  json_object* jsoLayer;
  for (int i = 0; i < nlayer; ++i)
  {
    jsoLayer = json_object_array_get_idx(layers, i);
    if (!SDL_strcmp(layerName, json_get_string(jsoLayer, "name")))
      return jsoLayer;
  }
  return NULL;
}

static int parse_tilelayer(const json_object* jsoTileLayer, tile_t* data)
{
  int                datalen;
  const json_object* jsaData;

  jsaData = json_object_object_get(jsoTileLayer, "data");
  datalen = json_object_array_length(jsaData);

  for (int i = 0; i < datalen; ++i)
    data[i] = json_array_get_int(jsaData, i);

  return 0;
}

/*
static void (*get_entity_create_fn(
    const char* entity_type_name))(ecs_Registry*, const EntityProperties*)
{
  static struct
  {
    const char* name;
    void (*const fn)(ecs_Registry*, const EntityProperties*);
  } lut[] = { { "Imp", parse_imp },
              { "Wogol", parse_wogol },
              { "BigDemon", parse_huge_demon },
              { "Chest", parse_chest },
              { "Ladder", parse_ladder },
              { "Chort", parse_chort },
              { "Door", parse_door } };

  for (int i = 0; lut[i].name != NULL; ++i)
    if (!SDL_strcmp(lut[i].name, entity_type_name))
      return lut[i].fn;
  return NULL;
}

static int parse_objectgroup(const json_object* jobjectgroup)
{
  const json_object* jobjects;
  int                objcnt;
  const char*        objtype;
  const json_object* jobj;

  EntityProperties params;
  void (*parse_fn)(ecs_Registry*, const EntityProperties*);

  jobjects = json_object_object_get(jobjectgroup, "objects");
  objcnt   = json_object_array_length(jobjects);

  for (int i = 0; i < objcnt; ++i)
  {
    jobj = json_object_array_get_idx(jobjects, i);

    // common attributes
    objtype           = json_get_string(jobj, "type");
    params.position.x = json_get_double(jobj, "x");
    params.position.y = json_get_double(jobj, "y");
    params.size.x     = json_get_double(jobj, "width");
    params.size.y     = json_get_double(jobj, "height");
    params.name       = json_get_string(jobj, "name");
    params.properties = json_object_object_get(jobj, "properties");
    params.id         = json_get_int(jobj, "id");

    if ((parse_fn = get_entity_create_fn(objtype)) != NULL)
      parse_fn(gRegistry, &params);
  }

  return 0;
}
*/

static int parse(const json_object* jmap)
{
  json_object *jsoLayers, /**jsoEttLayer,*/ *jsoWallLayer, *jsoFloorLayer;
  int          w, h;
  tile_t*      floor;
  tile_t*      wall;

  jsoLayers = json_object_object_get(jmap, "layers");
  w         = json_get_int(jmap, "width");
  h         = json_get_int(jmap, "height");

  floor = SDL_malloc(w * h * sizeof(tile_t));
  wall  = SDL_malloc(w * h * sizeof(tile_t));

  // jsoEttLayer = find_layer(jsoLayers, "entities");
  jsoWallLayer  = find_layer(jsoLayers, "wall");
  jsoFloorLayer = find_layer(jsoLayers, "floor");

  /*
  ASSERT(jsoEttLayer != NULL &&
         !SDL_strcmp(json_get_string(jsoEttLayer, "type"), "objectgroup"));
  */
  ASSERT(jsoFloorLayer != NULL &&
         !SDL_strcmp(json_get_string(jsoFloorLayer, "type"), "tilelayer"));
  ASSERT(jsoWallLayer != NULL &&
         !SDL_strcmp(json_get_string(jsoWallLayer, "type"), "tilelayer"));
  // parse_objectgroup(jsoEttLayer);
  parse_tilelayer(jsoFloorLayer, floor);
  parse_tilelayer(jsoWallLayer, wall);

  map_init(floor, wall, w, h);

  SDL_free(floor);
  SDL_free(wall);

  return 0;
}

int load_level(const char* levelName)
{
  char                filename[255];
  struct json_object* jsoMap;
  level_name_to_file_name(levelName, filename);

  if ((jsoMap = json_from_file(filename)) == NULL)
    return LOAD_LEVEL_FAIL_TO_LOAD_FILE;
  parse(jsoMap);
  json_object_put(jsoMap);
  return LOAD_LEVEL_OK;
}

/*
static Vec2 real_position(Vec2 top_left, Vec2 size)
{
  Vec2 result;
  result.x = top_left.x + size.x / 2.f;
  result.y = top_left.y + size.y;
  return result;
}
static void parse_imp(ecs_Registry* registry, const EntityProperties* props)
{
  make_imp(registry, real_position(props->position, props->size));
}
static void parse_wogol(ecs_Registry* registry, const EntityProperties* props)
{
  make_wogol(registry, real_position(props->position, props->size));
}
static void parse_huge_demon(ecs_Registry*           registry,
                             const EntityProperties* props)
{
  make_huge_demon(registry, real_position(props->position, props->size));
}
static void parse_chest(ecs_Registry* registry, const EntityProperties* props)
{
  MakeChestParams chest_params;
  chest_params.position = real_position(props->position, props->size);
  chest_params.id       = props->id;
  chest_params.state    = CHEST_STATE_CLOSE;

  parse_item_list(chest_params.items,
                  &chest_params.numSlots,
                  json_get_string(props->properties, "items"));
  make_chest(registry, &chest_params);
}

static int direction_from_string(const char* value)
{

  static struct
  {
    const char* string_value;
    u16         int_value;
  } lut[4] = { { "up", UP },
               { "down", DOWN },
               { "left", LEFT },
               { "right", RIGHT } };

  for (int i = 0; i < 4; ++i)
  {
    if (SDL_strcmp(value, lut[i].string_value) == 0)
      return lut[i].int_value;
  }

  ASSERT(FALSE && "invalid value");
  return -1;
}

static void parse_ladder(ecs_Registry* registry, const EntityProperties* props)
{
  const char* direction;

  direction = json_get_string(props->properties, "direction");
  MakePortalParams params;
  params.level     = json_get_string(props->properties, "level");
  params.dest      = json_get_string(props->properties, "dest");
  params.name      = props->name;
  params.position  = props->position;
  params.size      = props->size;
  params.direction = direction_from_string(direction);

  make_portal(registry, &params);
}

static void parse_chort(ecs_Registry* registry, const EntityProperties* params)
{
  make_chort(registry, real_position(params->position, params->size));
}

static void parse_door(ecs_Registry* registry, const EntityProperties* params)
{
  make_door(registry, real_position(params->position, params->size));
}
*/
