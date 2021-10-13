#include "components.h"
#include "equipment_system.h"

#define EQUIPMENT_SLOTS_INIT                                                                                   \
    {                                                                                                          \
        .weapon = EQM_NULL, .armor = EQM_NULL, .boots = EQM_NULL, .ring1 = EQM_NULL, .ring2 = EQM_NULL,        \
    }

#define EQUIPMENT_TYPE(id) (g_equipments[id].type)
#define EQUIPMENT_ATTACK_ACTION(id) (g_equipments[id].attack_action_1)
#define EQUIPMENT_ID_TO_ITEM_ID(id) (g_equipments[id].item_id)

const struct Equipment g_equipments[EQM_CNT] = {
    /*= WEAPONS =*/
    [EQM_ANIME_SWORD] = {
        .type = EQM_TYPE_WEAPON,
        .item_id = ITEM_ANIME_SWORD,
        .damage_type = DAMAGE_TYPE_PHYSICAL,
        .attack_action_1 = ATTACK_ACTION_SWING
    },

    /*= ARMORS  =*/
    [EQM_LEATHER_ARMOR] = {
        .type = EQM_TYPE_ARMOR,
        .item_id = ITEM_LEATHER_ARMOR,
        .modifier.defense.physical = 5,
        .modifier.health = 2,
    },

    /*= RINGS =*/
    [EQM_SAPPHIRE_RING] = {
        .type = EQM_TYPE_RING,
        .item_id = ITEM_SAPPHIRE_RING,
        .modifier.base.intelligent = 3,
        .modifier.defense.fire = 2,
        .modifier.health = 5,
    },
    [EQM_WOODEN_RING] = {
        .type = EQM_TYPE_RING,
        .item_id = ITEM_WOODEN_RING,
        .modifier.health = 2,
    },

    [EQM_RUBY_RING] = {
        .type = EQM_TYPE_RING,
        .item_id = ITEM_RUBY_RING,
        .modifier.defense.fire = 10,
    },

    /* BOOTS */
    [EQM_IRON_BOOTS] = {
        .type = EQM_TYPE_BOOTS,
        .item_id = ITEM_IRON_BOOTS,
        .modifier.defense.physical = 2
    },
};

static void
apply_stat_modifier(struct StatModifier* dst, u32 equipment_id)
{
    SDL_memcpy(dst, &g_equipments[equipment_id].modifier, sizeof *dst);
}

void
eqm_equip_weapon(ecs_Registry* registry, ecs_entity_t entity, u32 equipment_id)
{
    ASSERT_MSG(equipment_id < EQM_CNT, "Invalid equipment id");
    ASSERT_MSG(g_equipments[equipment_id].type == EQM_TYPE_WEAPON, "Given equipment is not a weapon");

    struct EquipmentSlotsComp* slots = ecs_assurev(registry, entity, EquipmentSlotsComp, EQUIPMENT_SLOTS_INIT);

    if (slots->weapon != equipment_id)
    {
        slots->weapon = equipment_id;

        // apply stats modifier
        struct StatModifiersComp* modifiers = ecs_get(registry, entity, StatModifiersComp);
        if (modifiers != NULL)
        {
            modifiers->equipments[EQM_SLOT_WEAPON].active = true;
            apply_stat_modifier(&modifiers->equipments[EQM_SLOT_WEAPON], equipment_id);
            ecs_assure(registry, entity, StatModifiersChangedTag);
        }

        // apply attack action
        switch (EQUIPMENT_ATTACK_ACTION(equipment_id))
        {
        case ATTACK_ACTION_SWING:
            ecs_assure(registry, entity, SwingWeaponComp);
            break;
        default:
            ASSERT_MSG(0, "Not implement yet");
        }
    }
}

static const enum EquipmentType s_compatiable_type[EQM_SLOT_CNT] = {
    EQM_TYPE_WEAPON, EQM_TYPE_ARMOR, EQM_TYPE_BOOTS, EQM_TYPE_RING, EQM_TYPE_RING,
};

#define COMPATIABLE_TYPE(slot) (s_compatiable_type[slot])

static void
equip_internal(ecs_Registry* registry, ecs_entity_t entity, enum EquipmentSlot slot, u32 equipment_id)
{
    ASSERT_MSG(equipment_id < EQM_CNT, "Invalid equipment id");
    ASSERT_MSG(EQUIPMENT_TYPE(equipment_id) == COMPATIABLE_TYPE(slot), "Given equipment is not compatiable");

    struct EquipmentSlotsComp* slots = ecs_assurev(registry, entity, EquipmentSlotsComp, EQUIPMENT_SLOTS_INIT);

    if (slots->as_array[slot] != equipment_id)
    {
        slots->as_array[slot] = equipment_id;

        struct StatModifiersComp* modifiers = ecs_get(registry, entity, StatModifiersComp);
        if (modifiers != NULL)
        {
            apply_stat_modifier(&modifiers->equipments[slot], equipment_id);
            modifiers->equipments[slot].active = true;
            ecs_assure(registry, entity, StatModifiersChangedTag);
        }
    }
}

void
eqm_equip_armor(ecs_Registry* registry, ecs_entity_t entity, u32 equipment_id)
{
    equip_internal(registry, entity, EQM_SLOT_ARMOR, equipment_id);
}

void
eqm_equip_boots(ecs_Registry* registry, ecs_entity_t entity, u32 equipment_id)
{
    equip_internal(registry, entity, EQM_SLOT_BOOTS, equipment_id);
}

void
eqm_equip_ring1(ecs_Registry* registry, ecs_entity_t entity, u32 equipment_id)
{
    equip_internal(registry, entity, EQM_SLOT_RING1, equipment_id);
}

void
eqm_equip_ring2(ecs_Registry* registry, ecs_entity_t entity, u32 equipment_id)
{
    equip_internal(registry, entity, EQM_SLOT_RING2, equipment_id);
}
