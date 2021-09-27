#include "graphics/material.h"
#include "graphics/gl.h"

void
material_init(Material* material, const Shader* shader, const MaterialPropertyInfo* infos, u32 count)
{
    ASSERT(count <= MATERIAL_MAX_PROPERTIES);
    ASSERT(shader != NULL);
    material->property_cnt = count;
    material->shader       = shader;

    if (count == 0)
        return;

    MaterialProperty*           prop;
    const MaterialPropertyInfo* info;
    const MaterialPropertyInfo* lastInfo = &infos[count];
    for (info = infos, prop = material->properties; info != lastInfo; ++prop, ++info)
    {
        prop->type     = info->type;
        prop->location = glGetUniformLocation(shader->handle, info->name);
        SDL_strlcpy(prop->name, info->name, (sizeof prop->name) - 1);
    }
}

void
material_set_int(Material* material, u32 prop_index, int value)
{
    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_INT);
    material->properties[prop_index].int_value = value;
}

void
material_set_float(Material* material, u32 prop_index, float value)
{

    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_FLOAT);
    material->properties[prop_index].float_value = value;
}

void
material_set_vec2(Material* material, u32 prop_index, vec2 value)
{
    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_VEC2);
    glm_vec2_copy(value, material->properties[prop_index].vec2_value);
}

void
material_set_vec3(Material* material, u32 prop_index, vec3 value)
{
    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_VEC3);
    glm_vec3_copy(value, material->properties[prop_index].vec2_value);
}

void
material_set_vec4(Material* material, u32 prop_index, vec4 value)
{
    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_VEC4);
    glm_vec4_copy(value, material->properties[prop_index].vec2_value);
}

void
material_set_texture(Material* material, u32 prop_index, const Texture* value)
{
    ASSERT(prop_index < material->property_cnt);
    ASSERT(material->properties[prop_index].type == MATERIAL_PROPERTY_TYPE_TEXTURE);
    material->properties[prop_index].texture_value = value;
}

void
material_use(const Material* material)
{
    const MaterialProperty* it;
    const MaterialProperty* end   = &material->properties[material->property_cnt];
    const MaterialProperty* begin = &material->properties[0];
    shader_bind(material->shader);

    for (it = begin; it != end; ++it)
    {
        switch (it->type)
        {
        case MATERIAL_PROPERTY_TYPE_INT:
            glUniform1i(it->location, it->int_value);
            break;
        case MATERIAL_PROPERTY_TYPE_FLOAT:
            glUniform1f(it->location, it->float_value);
            break;
        case MATERIAL_PROPERTY_TYPE_VEC2:
            glUniform1fv(it->location, 2, it->vec2_value);
            break;
        case MATERIAL_PROPERTY_TYPE_VEC3:
            glUniform1fv(it->location, 3, it->vec3_value);
            break;
        case MATERIAL_PROPERTY_TYPE_VEC4:
            glUniform1fv(it->location, 4, it->vec4_value);
            break;
        case MATERIAL_PROPERTY_TYPE_TEXTURE:
            ASSERT_MSG(0, "Not implemented yet!");
            break;
        case MATERIAL_PROPERTY_TYPE_UNDEFINED:
            ASSERT_MSG(0, "Invalid property type!");
        }
    }
}
