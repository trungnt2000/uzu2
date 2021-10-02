#include "entity.h"
#include "components.h"

static void
tag_transform_has_changed(ecs_Registry* registry, ecs_entity_t entity)
{
    ecs_add_or_set(registry, entity, TransformChangedTag, { 0 });
}

static void
tag_to_destroy(ecs_Registry* registry, ecs_entity_t entity)
{
    ecs_add_or_set(registry, entity, DestroyTag, { 0 });
}

void
ett_tx_set_position(ecs_Registry* registry, ecs_entity_t entity, vec3 position)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    glm_vec3_copy(position, t->position);
    tag_transform_has_changed(registry, entity);
}

void
ett_tx_set_rotation(ecs_Registry* registry, ecs_entity_t entity, float angle)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->rotation             = angle;
    tag_transform_has_changed(registry, entity);
}
void
ett_tx_set_scale(ecs_Registry* registry, ecs_entity_t entity, vec2 scale)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->scale[0]             = scale[0];
    t->scale[1]             = scale[1];
    tag_transform_has_changed(registry, entity);
}

void
ett_tx_rotate_by(ecs_Registry* registry, ecs_entity_t entity, float angle)
{
    struct TransformComp* t = ecs_get(registry, entity, TransformComp);
    t->rotation += angle;
    tag_transform_has_changed(registry, entity);
}

void
ett_rs_add_child(ecs_Registry* reg, ecs_entity_t ett, ecs_entity_t child)
{
    struct RelationshipComp* relationship = ecs_get_or_add(reg, ett, RelationshipComp);

    struct RelationshipComp* child_relationship =
        ecs_add_ex(reg, child, RelationshipComp, RELATIONSHIP_COMP_INIT_EMPTY);

    child_relationship->parent = ett;
    if (relationship->first != ECS_NULL_ENT)
    {
        struct RelationshipComp* first_child_relationship = ecs_get(reg, ett, RelationshipComp);
        first_child_relationship->prev                    = ett;
        child_relationship->next                          = relationship->first;
    }
    relationship->first = child;
}

bool
ett_rs_has_parent(ecs_Registry* reg, ecs_entity_t ett)
{
    return ett_rs_get_parent(reg, ett) != ECS_NULL_ENT;
}

ecs_entity_t
ett_rs_get_parent(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->parent : ECS_NULL_ENT;
}

ecs_entity_t
ett_rs_get_first_child(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->first : ECS_NULL_ENT;
}

ecs_entity_t
ett_rs_get_next_sibling(ecs_Registry* reg, ecs_entity_t ett)
{
    struct RelationshipComp* relationship = ecs_get(reg, ett, RelationshipComp);
    return relationship ? relationship->next : ECS_NULL_ENT;
}

ecs_entity_t
ett_rs_get_prev_sibling(ecs_Registry* reg, ecs_entity_t ett)
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

    vsnprintf(name_comp->value, sizeof(name_comp->value), fmt, va_arg);

    va_end(va_arg);
}
