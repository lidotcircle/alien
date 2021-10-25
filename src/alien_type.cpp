#include "alien_type.h"
#include "alien_type_basic.h"
#include "alien_type_struct.h"
#include "alien_type_union.h"
#include "alien_value.h"
#include <vector>
#include <string>
using namespace std;

#define ALIEN_TYPE_META "alien_type_metatable"
#define ALIEN_TYPE_TABLE "__alien_type_table"


static int alien_types_new(lua_State* L) {
    alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, 1, ALIEN_TYPE_META));
    alien_type* ptype = *pptype;

    alien_type_struct* stype = dynamic_cast<alien_type_struct*>(ptype);
    alien_type_union*  utype = dynamic_cast<alien_type_union*>(ptype);

    if (stype == nullptr && utype == nullptr)
        return luaL_error(L, "alien: only struct and union type can create values");

    alien_value_from_type(L, ptype);
    return 1;
}
static int alien_types_sizeof(lua_State* L) {
    alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, 1, ALIEN_TYPE_META));
    alien_type* ptype = *pptype;
    lua_pushnumber(L, ptype->__sizeof());
    return 1;
}
static int alien_types_gc(lua_State* L) {
    alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, 1, ALIEN_TYPE_META));
    alien_type* ptype = *pptype;
    delete ptype;
    return 0;
}
static int alien_types_tostring(lua_State* L) {
    alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, 1, ALIEN_TYPE_META));
    alien_type* ptype = *pptype;

    alien_type_struct* stype = dynamic_cast<alien_type_struct*>(ptype);
    alien_type_union*  utype = dynamic_cast<alien_type_union*>(ptype);
    if (stype != nullptr)
        lua_pushfstring(L, "[struct %s]", stype->__typename().c_str());

    if (utype != nullptr)
        lua_pushfstring(L, "[union %s]", utype->__typename().c_str());

    lua_pushfstring(L, "[%s]", ptype->__typename().c_str());
    return 1;
}

int alien_types_init(lua_State* L) {
    if (!lua_istable(L, 1))
        return luaL_error(L, "alien: register alien types failed, illegal argument");

    luaL_newmetatable(L, ALIEN_TYPE_META);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_types_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_types_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_newtable(L);
    lua_pushliteral(L, "new");
    lua_pushcfunction(L, alien_types_new);
    lua_settable(L, -3);
    lua_pushliteral(L, "sizeof");
    lua_pushcfunction(L, alien_types_sizeof);
    lua_settable(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_newtable(L);
    lua_setglobal(L, ALIEN_TYPE_TABLE);

    return 0;
}

int alien_types_register_basic(lua_State* L, const char* tname, ffi_type* ffitype) {
    lua_pushstring(L, tname);
    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, -2);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't define type '%s' twice", tname);
    lua_pop(L, 1);
    lua_pushvalue(L, -2);

    alien_type* new_type = new alien_type(tname, FFI_DEFAULT_ABI, ffitype);
    alien_type** udata = static_cast<alien_type**>(lua_newuserdata(L, sizeof(void*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *udata = new_type;

    lua_settable(L, -3);
    return 1;
}

/**
 * alien.defstruct(structname, {
 *     abi = abi || FFI_DEFAULT_ABI,
 *     { name1, member1 },
 *     { name2, member2 },
 *     ...
 *     { namen, membern },
 * }) => new_struct_type;
 */
int alien_types_defstruct(lua_State* L) {
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_istable(L, 2))
        return luaL_error(L, "alien: bad argument with 'defstruct( structname, definition )'");

    const char* structname = luaL_checkstring(L, 1);
    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't define type '%s' twice", structname);
    lua_pop(L, 1);

    ffi_abi abi = FFI_DEFAULT_ABI;
    lua_pushliteral(L, "abi");
    lua_gettable(L, 2);
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1))
            return luaL_error(L, "alien: bad abi");

        abi = static_cast<ffi_abi>(luaL_checkinteger(L, -1));
    }
    lua_pop(L, -1);

    vector<pair<string,alien_type*>> members;
    size_t nmember = luaL_len(L, 2);
    for(size_t i=1;i<=nmember;i++) {
        lua_rawgeti(L, 2, i);

        if (!lua_istable(L, -1) || luaL_len(L, -1) != 2)
            return luaL_error(L, "alien: defstruct bad member definition");

        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);

        const char* mname = luaL_checkstring(L, -2);
        alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, -1, ALIEN_TYPE_META));
        alien_type* ptype = *pptype;
        members.push_back(make_pair(mname, ptype));

        lua_pop(L, 3);
    }

    alien_type_struct* new_type = new alien_type_struct(structname, abi, members);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
}

/** similar with struct, see above */
int alien_types_defunion(lua_State* L) {
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_istable(L, 2))
        return luaL_error(L, "alien: bad argument with 'defunion( unionname, definition )'");

    const char* unionname = luaL_checkstring(L, 1);
    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't define type '%s' twice", unionname);
    lua_pop(L, 1);

    ffi_abi abi = FFI_DEFAULT_ABI;
    lua_pushliteral(L, "abi");
    lua_gettable(L, 2);
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1))
            return luaL_error(L, "alien: bad abi");

        abi = static_cast<ffi_abi>(luaL_checkinteger(L, -1));
    }
    lua_pop(L, -1);

    vector<pair<string,alien_type*>> members;
    size_t nmember = luaL_len(L, 2);
    for(size_t i=1;i<=nmember;i++) {
        lua_rawgeti(L, 2, i);

        if (!lua_istable(L, -1) || luaL_len(L, -1) != 2)
            return luaL_error(L, "alien: defunion bad member definition");

        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);

        const char* mname = luaL_checkstring(L, -2);
        alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, -1, ALIEN_TYPE_META));
        alien_type* ptype = *pptype;
        members.push_back(make_pair(mname, ptype));

        lua_pop(L, 3);
    }

    alien_type_union* new_type = new alien_type_union(unionname, abi, members);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
}

/** alias(newtypename, type) */
int alien_types_alias(lua_State* L) {
    const char* nname = luaL_checkstring(L, 1);
    luaL_checkudata(L, 2, ALIEN_TYPE_META);

    lua_getglobal(L, ALIEN_TYPE_TABLE);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't set alias '%s', type '%s' has existed", nname, nname);
    lua_pop(L, 1);

    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    return 0;
}

