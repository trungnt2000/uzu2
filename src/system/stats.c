
#include "components.h"
#include "ecs.h"

#define STAT_MODIFIER_ZERO_INIT                                                                                \
    {                                                                                                          \
        0                                                                                                      \
    }

static void
combine_stat_modifier(struct StatModifier* dst, const struct StatModifier* src)
{
    if (!src->active)
        return;

    dst->health += src->health;
    dst->mana += src->mana;
    dst->base.strength += src->base.strength;
    dst->base.intelligent += src->base.intelligent;
    dst->base.vitality += src->base.vitality;
    dst->base.luck += src->base.luck;

    dst->defense.physical += src->defense.physical;
    dst->defense.fire += src->defense.fire;
    dst->defense.lighting += src->defense.lighting;
    dst->defense.ice += src->defense.ice;
    dst->defense.holy += src->defense.holy;
    dst->defense.dark += src->defense.dark;
}

static void
compute_final_stat_modifier(ecs_Registry*        registry,
                            ecs_entity_t         entity,
                            struct StatModifier* final_stat_modifier)
{
    const struct StatModifiersComp* modifiers = ecs_get(registry, entity, StatModifiersComp);
    // entity does not have stats modifiers just leave it zero
    if (modifiers == NULL)
        return;

    for (int i = 0; i < EQM_SLOT_CNT; ++i)
    {
        combine_stat_modifier(final_stat_modifier, &modifiers->equipments[i]);
    }
    combine_stat_modifier(final_stat_modifier, &modifiers->buff);
    combine_stat_modifier(final_stat_modifier, &modifiers->debuff);
}

static void
update_final_stats(ecs_Registry* registry, const ecs_entity_t* entities, ecs_size_t count)
{
    struct HealthPoolComp*       health_pool;
    struct ManaPoolComp*         mana_pool;
    struct MagicalAtkPowComp*    mag;
    struct PhysicalAtkPowerComp* atk;
    struct DefenseComp*          defense;
    const struct BaseStatsComp*  stats;
    for (ecs_size_t i = 0; i < count; ++i)
    {
        struct StatModifier modifier = { 0 };
        compute_final_stat_modifier(registry, entities[i], &modifier);

        stats = ecs_get(registry, entities[i], BaseStatsComp);

        health_pool = ecs_get(registry, entities[i], HealthPoolComp);
        if (health_pool)
        {
            health_pool->max = stats->vitality * 2 + modifier.health;
        }

        mana_pool = ecs_get(registry, entities[i], ManaPoolComp);
        if (mana_pool)
        {
            mana_pool->max = stats->intelligent * 2 + modifier.mana;
        }

        mag = ecs_get(registry, entities[i], MagicalAtkPowComp);
        if (mag->value)
        {
            mag->value = stats->intelligent + modifier.base.intelligent;
        }

        atk = ecs_get(registry, entities[i], PhysicalAtkPowerComp);
        if (atk->value)
        {
            atk->value = stats->strength + modifier.base.strength;
        }
        defense = ecs_get(registry, entities[i], DefenseComp);

        if (defense != NULL)
        {
            s16 base = stats->vitality;
            s16 def  = stats->strength / 2;
            s16 res  = stats->intelligent / 2;

            defense->physical = base + modifier.defense.physical + def;
            defense->lighting = base + modifier.defense.lighting + res;
            defense->ice      = base + modifier.defense.ice + res;
            defense->fire     = base + modifier.defense.fire + res;
            defense->holy     = base + modifier.defense.holy + res;
            defense->dark     = base + modifier.defense.dark + res;
        }
    }
}

void
dispatch_base_stats_changed(ecs_Registry* registry)
{
    const ecs_entity_t* entities;
    ecs_size_t          count;
    ecs_raw(registry, BaseStatsChangedTag, &entities, NULL, &count);
    update_final_stats(registry, entities, count);
}

void
dispatch_stat_modifiers_changed(ecs_Registry* registry)
{
    const ecs_entity_t* entities;
    ecs_size_t          count;
    ecs_raw(registry, StatModifiersChangedTag, &entities, NULL, &count);
    update_final_stats(registry, entities, count);
}

void
system_stats_update(ecs_Registry* registry)
{
    dispatch_base_stats_changed(registry);
    dispatch_stat_modifiers_changed(registry);
}
