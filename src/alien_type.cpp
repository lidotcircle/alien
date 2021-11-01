#include "alien_type.h"
#include "alien_type_basic.h"
#include "alien_type_buffer.h"
#include "alien_type_callback.h"
#include "alien_type_pointer.h"
#include "alien_type_ref.h"
#include "alien_type_string.h"
#include "alien_type_struct.h"
#include "alien_type_union.h"
#include "alien_value.h"
#include "alien_function.h"
#include <assert.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <ffi.h>
using namespace std;

#define ALIEN_TYPE_META  "alien_type_metatable"
#define ALIEN_TYPE_TABLE "__alien_types"

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
    MENTRY( void,      ffi_type_void      ) \
    MENTRY( uint8_t,   ffi_type_uint8     ) \
    MENTRY( int8_t,    ffi_type_sint8     ) \
    MENTRY( uint16_t,  ffi_type_uint16    ) \
    MENTRY( int16_t,   ffi_type_sint16    ) \
    MENTRY( uint32_t,  ffi_type_uint32    ) \
    MENTRY( int32_t,   ffi_type_sint32    ) \
    MENTRY( uint64_t,  ffi_type_uint64    ) \
    MENTRY( int64_t,   ffi_type_sint64    ) \
    MENTRY( size_t,    ffi_type_size_t    ) \
    MENTRY( ptrdiff_t, ffi_type_ptrdiff_t ) \
    MENTRY( float,     ffi_type_float     ) \
    MENTRY( double,    ffi_type_double    )

#define type_alias_map \
    MENTRY( char,     int8_t   ) \
    MENTRY( byte,     uint8_t  ) \
    MENTRY( uchar,    uint8_t  ) \
    MENTRY( short,    int16_t  ) \
    MENTRY( ushort,   uint16_t ) \
    MENTRY( int,      int32_t  ) \
    MENTRY( unsigned, uint32_t ) \
    MENTRY( long,     int64_t  )


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
bool alien_type::is_rawpointer() const  { return false; }
bool alien_type::is_void() const  { return false; }
bool alien_type::is_basic() const   { return false; }
bool alien_type::is_ref() const     { return false; }
bool alien_type::is_pointer() const { return false; }
bool alien_type::is_string() const { return false; }
bool alien_type::is_struct() const  { return false; }
bool alien_type::is_buffer() const  { return false; }
bool alien_type::is_union() const   { return false; }
bool alien_type::is_callback() const{ return false; }

static bool alien_istype(lua_State* L, int idx)
{
    return luaL_testudata(L, idx, ALIEN_TYPE_META) != nullptr;
}

alien_type* alien_checktype(lua_State* L, int idx)
{
    auto t = static_cast<alien_type**>(luaL_checkudata(L, idx, ALIEN_TYPE_META));
    return *t;
}

int alien_push_type_table(lua_State* L) {
    alien_push_alien(L);
    lua_pushliteral(L, ALIEN_TYPE_TABLE);
    lua_gettable(L, -2);
    assert(lua_istable(L, -1));
    lua_remove(L, -2);
    return 1;
}

alien_type* alien_type_byname(lua_State* L, const char* name)
{
    alien_push_type_table(L);
    lua_pushstring(L, name);
    lua_gettable(L, -2);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return nullptr;
    } else {
        auto ans = alien_checktype(L, -1);
        lua_pop(L, 2);
        return ans;
    }
}

alien_type* alien_reftype(lua_State* L, alien_type* type) {
    if (type->is_ref())
        throw std::runtime_error("alien: type is already a reference");

    return nullptr;
}

alien_type* alien_ptrtype(lua_State* L, alien_type* type) {
    return nullptr;
}

static int alien_types_new(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);

    return alien_operator_method_new(L, type);
}
static int alien_types_value_is(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);
    if (lua_gettop(L) != 2)
        throw std::runtime_error("alien: value_is() takes exactly 2 arguments");
    bool isthis = type->is_this_type(L, 2);
    lua_pushboolean(L, isthis);
    return 1;
}
static int alien_types_sizeof(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);
    lua_pushnumber(L, type->__sizeof());
    return 1;
}
static int alien_types_gc(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);
    delete type;
    return 0;
}
static int alien_types_tostring(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);

    if (type->is_struct())
        lua_pushfstring(L, "[struct %s]", type->__typename().c_str());
    else if (type->is_union())
        lua_pushfstring(L, "[union %s]", type->__typename().c_str());
    else if (type->is_pointer())
        lua_pushfstring(L, "[pointer to %s]", type->__typename().c_str());
    else if (type->is_ref())
        lua_pushfstring(L, "[reference to %s]", type->__typename().c_str());
    else
        lua_pushfstring(L, "[%s]", type->__typename().c_str());
    return 1;
}

static int alien_types_register_basic(lua_State* L, const char* tname, ffi_type* ffitype) {
    lua_pushstring(L, tname);
    alien_push_type_table(L);
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
    lua_pop(L, 2);
    return 0;
}

int alien_types_init(lua_State* L) {
    auto n = lua_gettop(L);
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
    lua_pushliteral(L, "value_is");
    lua_pushcfunction(L, alien_types_value_is);
    lua_settable(L, -3);
    lua_pushliteral(L, "sizeof");
    lua_pushcfunction(L, alien_types_sizeof);
    lua_settable(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    alien_push_alien(L);
    lua_pushliteral(L, ALIEN_TYPE_TABLE);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

#define MENTRY(_a, _b) alien_types_register_basic(L, #_a, &_b);
    basic_type_map
#undef MENTRY

#define MENTRY(_s, _t) \
        lua_pushcfunction(L, alien_types_alias); \
        lua_pushstring(L, #_s); \
        lua_pushcfunction(L, alien_types_getbyname); \
        lua_pushstring(L, #_t); \
        lua_call(L, 1, 1); \
        lua_call(L, 2, 0);
    type_alias_map
#undef MENTRY

    alien_type* buftype = new alien_type_buffer();
    alien_type* strtype = new alien_type_string();
    alien_type* cbtype  = new alien_type_callback();
    vector<pair<string, alien_type*>> types = {
        {buftype->__typename(), buftype},
        {strtype->__typename(), strtype},
        {cbtype->__typename(),  cbtype}
    };

    alien_push_type_table(L);
    for(auto& t : types) {
        lua_pushstring(L, t.first.c_str());
        alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
        luaL_setmetatable(L, ALIEN_TYPE_META);
        *ud = t.second;
        lua_settable(L, -3);
    }
    lua_pop(L, 1);

    return 0;
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
    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't define type '%s' twice", structname);
    lua_pop(L, 1);

    ffi_abi abi = FFI_DEFAULT_ABI;
    lua_getfield(L, 2, "abi");
    if (!lua_isnil(L, -1))
        abi = alien_checkabi(L, -1);
    lua_pop(L, 1);

    vector<pair<string,alien_type*>> members;
    size_t nmember = luaL_len(L, 2);
    for(size_t i=1;i<=nmember;i++) {
        lua_rawgeti(L, 2, i);

        if (!lua_istable(L, -1) || luaL_len(L, -1) != 2)
            return luaL_error(L, "alien: defstruct bad member definition");

        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);

        const char* mname = luaL_checkstring(L, -2);
        alien_type* type = alien_checktype(L, -1);
        members.push_back(make_pair(mname, type));

        lua_pop(L, 3);
    }

    alien_type_struct* new_type = new alien_type_struct(structname, abi, members);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_type_table(L);
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
    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        return luaL_error(L, "alien: can't define type '%s' twice", unionname);
    lua_pop(L, 1);

    ffi_abi abi = FFI_DEFAULT_ABI;
    lua_getfield(L, 2, "abi");
    if (!lua_isnil(L, -1))
        abi = alien_checkabi(L, -1);
    lua_pop(L, 1);

    vector<pair<string,alien_type*>> members;
    size_t nmember = luaL_len(L, 2);
    for(size_t i=1;i<=nmember;i++) {
        lua_rawgeti(L, 2, i);

        if (!lua_istable(L, -1) || luaL_len(L, -1) != 2)
            return luaL_error(L, "alien: defunion bad member definition");

        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);

        const char* mname = luaL_checkstring(L, -2);
        alien_type* type = alien_checktype(L, -1);
        members.push_back(make_pair(mname, type));

        lua_pop(L, 3);
    }

    alien_type_union* new_type = new alien_type_union(unionname, abi, members);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
}

/** defref */
int alien_types_defref(lua_State* L) {
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !alien_istype(L, 2))
        return luaL_error(L, "alien: bad argument with 'defref( refname, type )'");

    const char* refname = luaL_checkstring(L, 1);
    alien_type* type = alien_checktype(L, 2);

    alien_type_ref* new_type = new alien_type_ref(type);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
}

/** defpointer */
int alien_types_defpointer(lua_State* L) {
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !alien_istype(L, 2))
        return luaL_error(L, "alien: bad argument with 'defpointer( pointername, type )'");

    const char* pointername = luaL_checkstring(L, 1);
    alien_type* type = alien_checktype(L, 2);

    alien_type_pointer* new_type = new alien_type_pointer(type);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
}

/** alias(newtypename, type) */
int alien_types_alias(lua_State* L) {
    const char* nname = luaL_checkstring(L, 1);
    if (!alien_istype(L, 2))
        return luaL_error(L, "alien: bad argument with 'alias(newtypename, type)'");

    alien_push_type_table(L);
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

/** atype(typename) */
int alien_types_getbyname(lua_State* L) {
    const char* nname = luaL_checkstring(L, 1);
    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);

    if (lua_isnil(L, -1))
        return luaL_error(L, "alien: type %s doesn't exist", nname);

    return 1;
}

