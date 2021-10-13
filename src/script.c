#include "script.h"
#include "entity.h"
#include "lauxlib.h"
#include "lualib.h"

static int
spt_lua_lib_ett_get_name(lua_State* L)
{
    struct spt_Entity handle = spt_lua_get_entity(L, -1);

    lua_pushstring(L, ett_get_name(handle.registry, handle.entity));

    return 1;
}

static int
spt_lua_lib_ett_set_name(lua_State* L)
{
    struct spt_Entity handle = spt_lua_get_entity(L, -2);
    const char*       name   = luaL_checkstring(L, -1);

    SDL_strlcpy(ecs_get(handle.registry, handle.entity, NameComp)->value, name, 50);
    return 0;
}

static int
spt_lua_lib_ett_get_position(lua_State* L)
{
    struct spt_Entity handle = spt_lua_get_entity(L, -1);

    const struct TransformComp* transform_comp = ecs_get(handle.registry, handle.entity, TransformComp);

    lua_pushnumber(L, transform_comp->position[0]);
    lua_pushnumber(L, transform_comp->position[1]);

    return 2;
}

static int
spt_lua_lib_ett_set_position(lua_State* L)
{
    struct spt_Entity handle = spt_lua_get_entity(L, -3);

    float x = (float)luaL_checknumber(L, -2);
    float y = (float)luaL_checknumber(L, -1);

    ett_tx_set_position(handle.registry, handle.entity, (vec3){ x, y, 1.f });
    return 0;
}

static luaL_Reg s_lua_lib_ett_m[] = {
    { "get_name", spt_lua_lib_ett_get_name },
    { "set_name", spt_lua_lib_ett_set_name },
    { "get_position", spt_lua_lib_ett_get_position },
    { "set_position", spt_lua_lib_ett_set_position },
};

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*a))
#define ARRAY_BEGIN(a) (&a[0])
#define ARRAY_END(a) (1 [&a])

static void
spt_lua_load_library_inl(lua_State* L, const char* type_name, const luaL_Reg* begin, const luaL_Reg* end)
{
    luaL_newmetatable(L, type_name);
    for (const luaL_Reg* it = begin; it != end; ++it)
    {
        lua_pushstring(L, it->name);
        lua_pushcfunction(L, it->func);
        lua_settable(L, -3);
    }
}

void
spt_lua_load_library_entity(lua_State* L)
{
    spt_lua_load_library_inl(L, "Entity", ARRAY_BEGIN(s_lua_lib_ett_m), ARRAY_END(s_lua_lib_ett_m));
}

void
spt_lua_push_entity(lua_State* L, struct spt_Entity entity)
{
    struct spt_Entity* handle = lua_newuserdata(L, sizeof *handle);

    luaL_getmetatable(L, "Entity");
    lua_setmetatable(L, -2);

    *handle = entity;
}

struct spt_Entity
spt_lua_get_entity(lua_State* L, int index)
{
    void* ud = luaL_checkudata(L, index, "Entity");
    luaL_argcheck(L, ud != NULL, 1, "`Entity` expected");
    return *(struct spt_Entity*)ud;
}
