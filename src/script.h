// ngotrung Mon 13 Sep 2021 06:32:32 PM +07
#ifndef SCRIPT_H
#define SCRIPT_H
#include "ecs.h"
#include "lua.h"

struct spt_Entity
{
    ecs_entity_t  entity;
    ecs_Registry* registry;
};

struct spt_EventScript
{
    lua_State* L;
} spt_EventScript;

void              spt_lua_push_entity(lua_State* L, struct spt_Entity entity);
struct spt_Entity spt_lua_get_entity(lua_State* L, int index);

lua_State* spt_load_lua_script(const char* file);

#endif // SCRIPT_H
