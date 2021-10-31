#include "alien_library.h"
#include "alien_function.h"
#include "alien_dep_lua.h"
#include "alien_type.h"
#include "alien_value.h"
#include "alien.h"
#include <assert.h>

#define MYNAME    "alien"
#define MYVERSION MYNAME " library for " LUA_VERSION " / " VERSION
#define ALIEN_LIBRARY_GLOBAL_REF "__alien__"

static const luaL_Reg alienlib[] = {
    {"load",         alien_load},
    {"functionlist", alien_functionlist},
    {"hasfunction",  alien_hasfunction},

    {"funcptr",      alien_function_new},

    {"defstruct",    alien_types_defstruct},
    {"defunion",     alien_types_defunion},
    {"defref",       alien_types_defref},
    {"defpointer",   alien_types_defpointer},
    {"alias",        alien_types_alias},
    {"atype",        alien_types_getbyname},
    {NULL, NULL},
};

extern "C" __EXPORT int luaopen_alien_c(lua_State *L) {
    /* Register main library */
    luaL_register(L, "alien", alienlib);

    lua_pushvalue(L, -1);
    lua_setglobal(L, ALIEN_LIBRARY_GLOBAL_REF);

    // version
    lua_pushliteral(L, MYVERSION);
    lua_setfield(L, -2, "version");

    // platform
    lua_pushliteral(L, PLATFORM);
    lua_setfield(L, -2, "platform");

    auto n = lua_gettop(L);

    alien_library_init (L);
    assert(lua_gettop(L) == n);

    alien_function_init(L);
    assert(lua_gettop(L) == n);

    alien_types_init(L);
    assert(lua_gettop(L) == n);

    alien_value_init(L);
    assert(lua_gettop(L) == n);

    // types
    alien_push_type_table(L);
    lua_setfield(L, -2, "types");

    assert(lua_istable(L, -1));
    return 1;
}

int alien_push_alien(lua_State* L) {
    lua_getglobal(L, ALIEN_LIBRARY_GLOBAL_REF);
    return 1;
}

