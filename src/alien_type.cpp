#include "alien_type.h"
#include "alien_type_basic.h"
#include "alien_type_struct.h"
#include "alien_type_union.h"
#include "alien_value.h"
#include <assert.h>
#include <vector>
#include <string>
using namespace std;

#define ALIEN_TYPE_META "alien_type_metatable"
#define ALIEN_TYPE_TABLE "__alien_type_table"

/* libffi extension to support size_t and ptrdiff_t */
#if PTRDIFF_MAX == 65535
# define ffi_type_size_t         ffi_type_uint16
# define ffi_type_ptrdiff_t      ffi_type_sint16
#elif PTRDIFF_MAX == 2147483647
# define ffi_type_size_t         ffi_type_uint32
# define ffi_type_ptrdiff_t      ffi_type_sint32
#elif PTRDIFF_MAX == 9223372036854775807
# define ffi_type_size_t         ffi_type_uint64
# define ffi_type_ptrdiff_t      ffi_type_sint64
#elif defined(_WIN64)
# define ffi_type_size_t         ffi_type_uint64
# define ffi_type_ptrdiff_t      ffi_type_sint64
#elif defined(WINDOWS)
# define ffi_type_size_t         ffi_type_uint32
# define ffi_type_ptrdiff_t      ffi_type_sint32
#else
 #error "ptrdiff_t size not supported"
#endif

#define basic_type_map \
        MENTRY( void      ) \
        MENTRY( uint8_t   ) \
        MENTRY( int8_t    ) \
        MENTRY( uint16_t  ) \
        MENTRY( int16_t   ) \
        MENTRY( uint32_t  ) \
        MENTRY( int32_t   ) \
        MENTRY( uint64_t  ) \
        MENTRY( int64_t   ) \
        MENTRY( size_t    ) \
        MENTRY( ptrdiff_t ) \
        MENTRY( float     ) \
        MENTRY( double    ) \

alien_type::alien_type(const string& tname): __tname(tname) {}
const ffi_type* alien_type::ffitype() const {
    return const_cast<alien_type*>(this)->ffitype(); 
}
const string& alien_type::__typename() const {
    return this->__tname;
}
size_t alien_type::__sizeof() const {
    return this->ffitype()->size;
}
bool alien_type::is_integer() const { return false; }
bool alien_type::is_signed() const  { return false; }
bool alien_type::is_float() const   { return false; }
bool alien_type::is_double() const  { return false; }
bool alien_type::is_basic() const   { return false; }
bool alien_type::is_ref() const     { return false; }
bool alien_type::is_pointer() const { return false; }
bool alien_type::is_struct() const  { return false; }
bool alien_type::is_buffer() const  { return false; }
bool alien_type::is_union() const   { return false; }

static int alien_types_new(lua_State* L) {
    alien_type** pptype = static_cast<alien_type**>(luaL_checkudata(L, 1, ALIEN_TYPE_META));
    alien_type* ptype = *pptype;

    return alien_operator_method_new(L, ptype);
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

    if (ptype->is_struct())
        lua_pushfstring(L, "[struct %s]", ptype->__typename().c_str());
    else if (ptype->is_union())
        lua_pushfstring(L, "[union %s]", ptype->__typename().c_str());
    else if (ptype->is_pointer())
        lua_pushfstring(L, "[pointer to %s]", ptype->__typename().c_str());
    else if (ptype->is_ref())
        lua_pushfstring(L, "[reference to %s]", ptype->__typename().c_str());
    else
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

    alien_types_register_basic(L, "int", &ffi_type_uint);

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

    alien_type* new_type = new alien_type_basic(tname, FFI_DEFAULT_ABI, ffitype);
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

