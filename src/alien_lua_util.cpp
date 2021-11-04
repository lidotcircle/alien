#include "alien_lua_util.h"
#include "alien_exception.h"

#define ALIEN_ROTABLE_META "alien_rotable_meta"


ALIEN_LUA_FUNC static int alien_rotable_gc(lua_State* L);
ALIEN_LUA_FUNC static int alien_rotable_tostring(lua_State* L);
ALIEN_LUA_FUNC static int alien_rotable_index(lua_State* L);
ALIEN_LUA_FUNC static int alien_rotable_newindex(lua_State* L);

int alien_rotable_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_ROTABLE_META);

    lua_pushcfunction(L, alien_rotable_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, alien_rotable_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, alien_rotable_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, alien_rotable_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pop(L, 1);
    return 0;
}

ALIEN_LUA_FUNC int alien_rotable_new(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    void* ax = lua_newuserdata(L, sizeof(void*));
    *static_cast<void**>(ax) = nullptr;
    luaL_setmetatable(L, ALIEN_ROTABLE_META);
    lua_newtable(L);
    lua_setuservalue(L, -2);
    return 1;
    ALIEN_EXCEPTION_END();
}

bool alien_isrotable(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_ROTABLE_META) != nullptr;
}

int alien_rotable_rawtable(lua_State* L, int idx) {
    luaL_checkudata(L, idx, ALIEN_ROTABLE_META);
    lua_getuservalue(L, idx);
    return 1;
}

static int alien_rotable_gc(lua_State* L) {
    return 0;
}
static int alien_rotable_tostring(lua_State* L) {
    lua_pushstring(L, "[rotable]");
    return 1;
}
ALIEN_LUA_FUNC static int alien_rotable_index(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_rotable_rawtable(L, 1);
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);
    return 1;
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_rotable_newindex(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    throw AlienException("can't modify readonly table");
    ALIEN_EXCEPTION_END();
}

