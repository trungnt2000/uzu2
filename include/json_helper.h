#ifndef JSON_HELPER_H
#define JSON_HELPER_H
#include "toolbox.h"
#include "json-c/json.h"


#define json_array_get_int(a, i)\
  json_object_get_int(json_object_array_get_idx(a, i))

#define json_array_get_string(a, i)\
  json_object_get_string(json_object_array_get_idx(a, i))

#define json_get_string(j, k)\
  json_object_get_string(json_object_object_get(j, k))

#define json_get_int(j, k)\
  json_object_get_int(json_object_object_get(j, k))

#define json_get_double(j, k)\
  json_object_get_double(json_object_object_get(j, k))

#define json_get_object(j, k)\
  json_object_object_get(j, k)

json_object* json_from_file(const char* filename);

#endif // JSON_HELPER_H
