// ngotrung Mon 13 Sep 2021 06:32:32 PM +07
#ifndef SCRIPT_H
#define SCRIPT_H
#include "ecs.h"
#include "lua.h"

typedef struct spt_Entity
{
    ecs_entity_t  entity;
    ecs_Registry* registry;
} spt_Entity;

typedef struct spt_EventScript
{
    lua_State* vm;
} spt_EventScript;

enum spt_lvm_Library
{
    SPT_LVM_LIB_ENTITY = 0,
    SPT_LVM_LIB_ITEM   = 1
};

void       spt_lvm_push_entity(lua_State* vm, struct spt_Entity entity);
spt_Entity spt_lvm_get_entity(lua_State* vm, struct spt_Entity entity);

lua_State* spt_load_lua_script(const char* file, int libs);

#endif // SCRIPT_H
