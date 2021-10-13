#include "entity.h"
#include "components.h"

static void
tag_transform_has_changed(ecs_Registry* registry, ecs_entity_t entity)
{
    ecs_assure(registry, entity, TransformChangedTag);
}

static void
tag_to_destroy(ecs_Registry* registry, ecs_entity_t entity)
{
    ecs_assure(registry, entity, DestroyTag);
}

void
ett_set_position(ecs_Registry* registry, ecs_entity_t entity, vec3 position)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    glm_vec3_copy(position, t->position);
    tag_transform_has_changed(registry, entity);
}

void
ett_set_rotation(ecs_Registry* registry, ecs_entity_t entity, float angle)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->rotation             = angle;
    tag_transform_has_changed(registry, entity);
}
void
ett_set_scale(ecs_Registry* registry, ecs_entity_t entity, vec2 scale)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->scale[0]             = scale[0];
    t->scale[1]             = scale[1];
    tag_transform_has_changed(registry, entity);
}

void
ett_rotate_by(ecs_Registry* registry, ecs_entity_t entity, float angle)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->rotation += angle;
    tag_transform_has_changed(registry, entity);
}

ecs_entity_t
ett_add_child(ecs_Registry* reg, ecs_entity_t ett, ecs_entity_t child)
{
    struct RelationshipComp* relationship = ecs_assurev(reg, ett, RelationshipComp, RELATIONSHIP_COMP_INIT);

    struct RelationshipComp* child_relationship =
        ecs_assurev(reg, child, RelationshipComp, RELATIONSHIP_COMP_INIT);

    child_relationship->parent = ett;
    if (relationship->first != ECS_NULL_ENT)
    {
        struct RelationshipComp* first_child_relationship = ecs_get(reg, ett, RelationshipComp);
        first_child_relationship->prev                    = ett;
        child_relationship->next                          = relationship->first;
    }
    relationship->first = child;
    return child;
}

bool
ett_has_parent(ecs_Registry* reg, ecs_entity_t ett)
{
    return ett_get_parent(reg, ett) != ECS_NULL_ENT;
}

ecs_entity_t
ett_get_parent(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->parent : ECS_NULL_ENT;
}

ecs_entity_t
ett_get_first_child(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->first : ECS_NULL_ENT;
}

ecs_entity_t
ett_get_next_sibling(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->next : ECS_NULL_ENT;
}

ecs_entity_t
ett_get_prev_sibling(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->prev : ECS_NULL_ENT;
}

#include <stdarg.h>
void
ett_set_name_fmt(ecs_Registry* reg, ecs_entity_t ett, const char* restrict fmt, ...)
{
    va_list va_arg;
    va_start(va_arg, fmt);

    struct NameComp* name_comp = ecs_get(reg, ett, NameComp);

    if (!name_comp)
        name_comp = ecs_add(reg, ett, NameComp);

    vsnprintf(name_comp->value, sizeof(name_comp->value) - 1, fmt, va_arg);

    va_end(va_arg);
}

void
ett_attach_weapon(ecs_Registry* registry, ecs_entity_t entity, ecs_entity_t weapon)
{
    struct RefComp* ref_comp = ecs_get(registry, entity, RefComp);
    ASSERT_MSG(ref_comp, "Unable to equip weapon");
    ref_comp->weapon = weapon;

    ett_set_position(registry, weapon, (vec3){ 3.f, 0.f, 0.f });
    ett_add_child(registry, ref_comp->hand, weapon);
}
