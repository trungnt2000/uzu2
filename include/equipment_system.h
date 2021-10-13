// ngotrung Sat 09 Oct 2021 11:33:59 PM +07
#ifndef EQUIPMENT_SYSTEM_H
#define EQUIPMENT_SYSTEM_H

#include "components.h"
#include "ecs.h"
#include "global.h"


struct Equipment
{
    enum EquipmentType  type;
    enum ItemId         item_id;         /* associated item id               */
    enum DamageType     damage_type;     /* physical weapon only             */
    enum AttackAction   attack_action_1; /* weapon only                      */
    enum AttackAction   attack_action_2; /* weapon only                      */
    s16                 attack_power;    /* weapon only                      */
    struct StatModifier modifier;        /*                                  */
    u32                 flags;           /* determine which job can use      */
};

void eqm_equip_weapon(ecs_Registry* registry, ecs_entity_t entity, u32 eqm_id);
void eqm_equip_armor(ecs_Registry* registry, ecs_entity_t entity, u32 eqm_id);
void eqm_equip_boots(ecs_Registry* registry, ecs_entity_t entity, u32 eqm_id);
void eqm_equip_ring1(ecs_Registry* registry, ecs_entity_t entity, u32 eqm_id);
void eqm_equip_ring2(ecs_Registry* registry, ecs_entity_t entity, u32 eqm_id);

void eqm_unequip_weapon(ecs_Registry* registry, ecs_entity_t entity);
void eqm_unequip_amour(ecs_Registry* registry, ecs_entity_t entity);
void eqm_unequip_boots(ecs_Registry* registry, ecs_entity_t entity);
void eqm_unequip_ring1(ecs_Registry* registry, ecs_entity_t entity);
void eqm_unequip_ring2(ecs_Registry* registry, ecs_entity_t entity);

void emp_remove_all(ecs_Registry* registry, ecs_entity_t entity);

extern const struct Equipment g_equipments[EQM_CNT];

#define EQUIPMENT_ICON(id) (item_icon(g_equipments[id].item_id))
#define EQUIPMENT_NAME(id) (item_name(g_equipments[id].item_id))
#endif // EQUIPMENT_SYSTEM_H
