// ngotrung Thu 19 Aug 2021 12:45:44 AM +07
#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H
#include "graphics/types.h"

typedef enum MaterialPropertyType
{
  MATERIAL_PROPERTY_TYPE_UNDEFINED,
  MATERIAL_PROPERTY_TYPE_INT,
  MATERIAL_PROPERTY_TYPE_FLOAT,
  MATERIAL_PROPERTY_TYPE_VEC2,
  MATERIAL_PROPERTY_TYPE_VEC3,
  MATERIAL_PROPERTY_TYPE_VEC4,
  MATERIAL_PROPERTY_TYPE_TEXTURE
} MaterialPropertyType;

typedef struct MaterialProperty
{
  char                 name[32];
  int                  location; /* uniform location */
  MaterialPropertyType type;
  union
  {
    int            int_value;
    float          float_value;
    vec2           vec2_value;
    vec3           vec3_value;
    vec4           vec4_value;
    const Texture* texture_value;
  };
} MaterialProperty;

typedef struct MaterialPropertyInfo
{
  char                 name[32];
  MaterialPropertyType type;
} MaterialPropertyInfo;

#define MATERIAL_MAX_PROPERTIES 8

typedef struct Material
{
  const Shader* shader;
  u32                 property_cnt;
  MaterialProperty    properties[MATERIAL_MAX_PROPERTIES];
} Material;

void
material_init(Material* material, const Shader* shader, const MaterialPropertyInfo* infos, u32 count);

void material_set_int(Material* material, u32 propIndex, int value);
void material_set_float(Material* material, u32 propIndex, float value);
void material_set_vec2(Material* material, u32 propIndex, vec2 value);
void material_set_vec3(Material* material, u32 propIndex, vec3 value);
void material_set_vec4(Material* material, u32 propIndex, vec4 value);
void material_set_texture(Material* material, u32 propIndex, const Texture* value);

void material_use(const Material* material);

#endif // GRAPHICS_MATERIAL_H
