#include "alien_library.h"
#include "alien_function.h"
#include "alien_dep_lua.h"
#include "alien_type.h"
#include "alien_value.h"
#include "alien.h"

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
/*
    {"align",        alien_align},
    {"tag",          alien_register},
    {"wrap",         alien_wrap},
    {"rewrap",       alien_rewrap},
    {"unwrap",       alien_unwrap},
    {"tostring",     alien_udata2str},
    {"isnull",       alien_isnull},
    {"sizeof",       alien_sizeof},
    {"tochar",       alien_udata2char},
    {"toshort",      alien_udata2short},
    {"toushort",     alien_udata2ushort},
    {"toint",        alien_udata2int},
    {"touint",       alien_udata2uint},
    {"tolong",       alien_udata2long},
    {"toulong",      alien_udata2ulong},
    {"toptrdiff_t",  alien_udata2ptrdiff_t},
    {"tosize_t",     alien_udata2size_t},
    {"tofloat",      alien_udata2float},
    {"todouble",     alien_udata2double},
    {"buffer",       alien_buffer_new},
    {"callback",     alien_callback_new},
    {"table",        alien_table_new},
    {"errno",        alien_errno},
    {"memmove",      alien_memmove},
    {"memset",       alien_memset},
*/

    {NULL, NULL},
};

extern "C" __EXPORT int luaopen_alien_c(lua_State *L) {
    /* Register main library */
    luaL_register(L, "alien", alienlib);

    lua_pushvalue(L, -1);
    lua_setglobal(L, ALIEN_LIBRARY_GLOBAL_REF);

    /* Version */
    lua_pushliteral(L, MYVERSION);
    lua_setfield(L, -2, "version");

    /* Set platform */
    lua_pushliteral(L, PLATFORM);
    lua_setfield(L, -2, "platform");

    alien_library_init (L);
    alien_function_init(L);
    alien_types_init   (L);
    alien_value_init   (L);

    return 1;
}

int alien_push_alien(lua_State* L) {
    lua_getglobal(L, ALIEN_LIBRARY_GLOBAL_REF);
    return 1;
}

