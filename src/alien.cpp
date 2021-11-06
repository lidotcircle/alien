#include "alien_library.h"
#include "alien_function.h"
#include "alien_dep_lua.h"
#include "alien_type.h"
#include "alien_value.h"
#include "alien_value_callback.h"
#include "alien.h"
#include "alien_lua_util.h"
#include <assert.h>

#define MYNAME    "alien"
#define MYVERSION MYNAME " library for " LUA_VERSION " / " VERSION
#define ALIEN_LIBRARY_GLOBAL_REF "__alien__"

static const luaL_Reg alienlib[] = {
    {NULL, NULL},
};

extern "C" __EXPORT int luaopen_alien_c(lua_State *L) {
    /* Register main library */
    luaL_register(L, "alien", alienlib);

    return 1;
}

int alien_push_alien(lua_State* L) {
    lua_getglobal(L, ALIEN_LIBRARY_GLOBAL_REF);
    return 1;
}

