#include "alien_type.h"
#include "alien_type_basic.h"
#include "alien_type_buffer.h"
#include "alien_type_callback.h"
#include "alien_type_pointer.h"
#include "alien_type_ref.h"
#include "alien_type_string.h"
#include "alien_type_struct.h"
#include "alien_type_union.h"
#include "alien_type_array.h"
#include "alien_value.h"
#include "alien_function.h"
#include "alien_lua_util.h"
#include "alien_exception.h"
#include <ctype.h>
#include <assert.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <map>
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
    MENTRY( void,       ffi_type_void      ) \
    MENTRY( uint8_t,    ffi_type_uint8     ) \
    MENTRY( int8_t,     ffi_type_sint8     ) \
    MENTRY( uint16_t,   ffi_type_uint16    ) \
    MENTRY( int16_t,    ffi_type_sint16    ) \
    MENTRY( uint32_t,   ffi_type_uint32    ) \
    MENTRY( int32_t,    ffi_type_sint32    ) \
    MENTRY( uint64_t,   ffi_type_uint64    ) \
    MENTRY( int64_t,    ffi_type_sint64    ) \
    MENTRY( size_t,     ffi_type_size_t    ) \
    MENTRY( ptrdiff_t,  ffi_type_ptrdiff_t ) \
    MENTRY( float,      ffi_type_float     ) \
    MENTRY( double,     ffi_type_double    ) \
    MENTRY( rawpointer, ffi_type_pointer   )

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
bool alien_type::is_array() const   { return false; }
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
    lua_remove(L, -2);
    return 1;
}

static int alien_push_raw_type_table(lua_State* L) {
    alien_push_type_table(L);
    alien_rotable_rawtable(L, -1);
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
        throw AlienException("type is already a reference");

    return nullptr;
}

alien_type* alien_ptrtype(lua_State* L, alien_type* type) {
    return nullptr;
}

static int alien_types_register_basic(lua_State* L, const char* tname, ffi_type* ffitype) {
    lua_pushstring(L, tname);
    alien_push_raw_type_table(L);
    lua_pushvalue(L, -2);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        throw AlienException("can't define type '%s' twice", tname);
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

ALIEN_LUA_FUNC static int alien_types_gc(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_tostring(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_index(lua_State* L);

int alien_types_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_TYPE_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_types_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_types_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_types_index);
    lua_settable(L, -3);
    lua_pop(L, 1);

    alien_push_alien(L);
    lua_pushliteral(L, ALIEN_TYPE_TABLE);
    alien_rotable_new(L);
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
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) { \
            const char* error = lua_tostring(L, -1); \
            throw AlienLuaThrow("%s", error); \
        } \
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) { \
            const char* error = lua_tostring(L, -1); \
            throw AlienLuaThrow("%s", error); \
        }
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

    alien_push_raw_type_table(L);
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

ALIEN_LUA_FUNC static int alien_types_new(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_value_is(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_sizeof(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_box(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_ptr(lua_State* L);
ALIEN_LUA_FUNC static int alien_types_ref(lua_State* L);

const map<string, lua_CFunction> alien_types_methods = {
    {"new",      alien_types_new},
    {"value_is", alien_types_value_is},
    {"sizeof",   alien_types_sizeof},
    {"box",      alien_types_box},
};
const map<string, lua_CFunction> alien_types_properties = {
    {"ptr", alien_types_ptr},
    {"ref", alien_types_ref},
};
ALIEN_LUA_FUNC static int alien_types_index(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);
    const string key(luaL_checkstring(L, 2));

    if (alien_types_methods.find(key) != alien_types_methods.end()) {
        lua_pushcfunction(L, alien_types_methods.at(key));
        return 1;
    } else if (alien_types_properties.find(key) != alien_types_properties.end()) {
        lua_pushcfunction(L, alien_types_properties.at(key));
        lua_pushvalue(L, 1);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            throw AlienLuaThrow("%s", error);
        }
        return 1;
    } else {
        throw AlienException("unknown type property '%s'", key.c_str());
    }
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_types_new(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);

    return alien_operator_method_new(L, type);
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_types_value_is(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);
    if (lua_gettop(L) != 2)
        throw AlienInvalidArgumentException("value_is() takes exactly 2 arguments");
    bool isthis = type->is_this_type(L, 2);
    lua_pushboolean(L, isthis);
    return 1;
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_types_sizeof(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);
    lua_pushnumber(L, type->__sizeof());
    return 1;
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_types_gc(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);
    delete type;
    return 0;
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_types_box(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type_basic* type = dynamic_cast<alien_type_basic*>(alien_checktype(L, 1));
    if (!type || !type->is_basic())
        throw AlienInvalidArgumentException("cannot box non-basic type");
    if (lua_gettop(L) != 2)
        throw AlienInvalidArgumentException("box() takes exactly 2 arguments");

    int n = type->box(L, 2);
    assert(n == 1);
    return 1;
    ALIEN_EXCEPTION_END();
}
ALIEN_LUA_FUNC static int alien_types_tostring(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_type* type = alien_checktype(L, 1);

    if (type->is_struct())
        lua_pushfstring(L, "[struct %s]", type->__typename().c_str());
    else if (type->is_union())
        lua_pushfstring(L, "[union %s]", type->__typename().c_str());
    else
        lua_pushfstring(L, "[%s]", type->__typename().c_str());
    return 1;
    ALIEN_EXCEPTION_END();
}

static pair<alien_type*,string> ensure_type (lua_State* L, const string& type_n);
static pair<alien_type*,string> ensure_refto(lua_State* L, const string& type_n);
static pair<alien_type*,string> ensure_array(lua_State* L, const string& type_n, size_t n);
static pair<alien_type*,string> ensure_ptrto(lua_State* L, const string& type_n);

static pair<alien_type*,string> ensure_type(lua_State* L, const string& type_n) 
{
    if (type_n.empty())
        return make_pair(nullptr, "alien: illegal type name '" + type_n + "' <1>");

    alien_push_raw_type_table(L);
    lua_getfield(L, -1, type_n.c_str());
    if (!lua_isnil(L, -1)) {
        auto ans = alien_checktype(L, -1);
        lua_pop(L, 2);
        return make_pair(ans, "");
    }
    lua_pop(L, 2);

    char lastchar = type_n.back();
    if (lastchar == '*') {
        return ensure_ptrto(L, type_n.substr(0, type_n.size()-1));
    } else if (lastchar == '&') {
        return ensure_refto(L, type_n.substr(0, type_n.size()-1));
    } else if (lastchar == ']') {
        size_t n = type_n.size() - 1;
        while(n > 0 && type_n[n-1] != '[')
            --n;

        if (n == 0)
            return make_pair(nullptr, "alien: illegal type name '" + type_n + "' <2>");

        n--;
        auto t = type_n.substr(0, n);
        auto s = type_n.substr(n + 1, type_n.size()-n-2);
        size_t sl = 0;
        int array_size = std::stoi(s, &sl, 0);
        if (array_size <= 0 || sl != s.size())
            return make_pair(nullptr, "alien: illegal type name '" + type_n + "' <3>");

        return ensure_array(L, t, array_size);
    } else {
        return make_pair(nullptr, "alien: type '" + type_n + "' not defined");
    }
}

static pair<alien_type*,string> ensure_refto(lua_State* L, const string& type_n) 
{
    alien_push_raw_type_table(L);
    string new_typename = type_n + "&";
    lua_getfield(L, -1, new_typename.c_str());
    if (!lua_isnil(L, -1)) {
        auto ans = alien_checktype(L, -1);
        lua_pop(L, 2);
        return make_pair(ans, "");
    }
    lua_pop(L, 2);

    alien_type* tx = nullptr;
    auto enret = ensure_type(L, type_n);
    if (enret.first == nullptr)
        return enret;
    tx = enret.first;

    if (tx->is_ref()) {
        return make_pair(nullptr, "alien: type is already a reference");
    } else if (tx->is_buffer()) {
        return make_pair(nullptr, "alien: can't make reference to buffer");
    } else if (tx->is_callback()) {
        return make_pair(nullptr, "alien: can't make reference to callback");
    } else if (tx->is_string()) {
        return make_pair(nullptr, "alien: can't make reference to string");
    } else if (tx->is_void()) {
        return make_pair(nullptr, "alien: can't make reference to void");
    }

    alien_type_ref* new_type = new alien_type_ref(tx);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_raw_type_table(L);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, new_typename.c_str());
    lua_pop(L, 2);

    return make_pair(new_type, "");
}

static pair<alien_type*,string> ensure_ptrto(lua_State* L, const string& type_n) 
{
    alien_push_raw_type_table(L);
    string new_typename = type_n + "*";
    lua_getfield(L, -1, new_typename.c_str());
    if (!lua_isnil(L, -1)) {
        auto ans = alien_checktype(L, -1);
        lua_pop(L, 2);
        return make_pair(ans, "");
    }
    lua_pop(L, 2);

    alien_type* tx = nullptr;
    auto enret = ensure_type(L, type_n);
    if (enret.first == nullptr)
        return enret;
    tx = enret.first;

    if (tx->is_ref())
        return make_pair(nullptr, "alien: cna't make pointer to reference");
    else if (tx->is_buffer())
        return make_pair(nullptr, "alien: can't make pointer to buffer");
    else if (tx->is_callback())
        return make_pair(nullptr, "alien: can't make pointer to callback");
    else if (tx->is_string())
        return make_pair(nullptr, "alien: can't make pointer to string");
    else if (tx->is_void())
        return make_pair(nullptr, "alien: can't make pointer to void");

    alien_type_pointer* new_type = new alien_type_pointer(tx);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    alien_push_raw_type_table(L);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, new_typename.c_str());
    lua_pop(L, 2);

    return make_pair(new_type, "");
}

static pair<alien_type*,string> ensure_array(lua_State* L, const string& type_n, size_t n)
{
    alien_push_raw_type_table(L);
    string new_typename = type_n + "[" + std::to_string(n) + "]";
    lua_getfield(L, -1, new_typename.c_str());
    if (!lua_isnil(L, -1)) {
        auto ans = alien_checktype(L, -1);
        lua_pop(L, 2);
        return make_pair(ans, "");
    }
    lua_pop(L, 2);

    alien_type* tx = nullptr;
    auto enret = ensure_type(L, type_n);
    if (enret.first == nullptr)
        return enret;
    tx = enret.first;

    if (tx->is_ref())
        return make_pair(nullptr, "alien: cna't make array to reference");
    else if (tx->is_buffer())
        return make_pair(nullptr, "alien: can't make array to buffer");
    else if (tx->is_callback())
        return make_pair(nullptr, "alien: can't make array to callback");
    else if (tx->is_string())
        return make_pair(nullptr, "alien: can't make array to string");
    else if (tx->is_void())
        return make_pair(nullptr, "alien: can't make array to void");

    alien_type_array* new_type = new alien_type_array(tx, n, FFI_DEFAULT_ABI);
    alien_type** ud = static_cast<alien_type**>(lua_newuserdata(L, sizeof(alien_type*)));
    luaL_setmetatable(L, ALIEN_TYPE_META);
    *ud = new_type;

    std::stringstream ss;
    ss << type_n << "[0x" << std::hex << n << "]";
    string new_typename_hex = ss.str();

    alien_push_raw_type_table(L);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, new_typename.c_str());
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, new_typename_hex.c_str());
    lua_pop(L, 2);

    return make_pair(new_type, "");
}

static int alien_types_ref(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);
    auto nm = ensure_refto(L, type->__typename());
    if (nm.first == nullptr)
        throw AlienException("%s", nm.second.c_str());

    alien_push_type_table(L);
    lua_getfield(L, -1, (type->__typename() + '&').c_str());
    return 1;
}

static int alien_types_ptr(lua_State* L) {
    alien_type* type = alien_checktype(L, 1);
    auto nm = ensure_ptrto(L, type->__typename());
    if (nm.first == nullptr)
        throw AlienException("%s", nm.second.c_str());

    alien_push_type_table(L);
    lua_getfield(L, -1, (type->__typename() + '*').c_str());
    return 1;
}

static bool validate_typename(const string& type_name)
{
    if (type_name.empty()) return false;
    auto c1 = type_name[0];

    if (c1 != '_' && !isalpha(c1))
        return false;

    for (auto c : type_name) {
        if (c == '_') continue;
        if (isalnum(c)) continue;

        return false;
    }

    return true;
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
ALIEN_LUA_FUNC int alien_types_defstruct(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_istable(L, 2))
        throw AlienInvalidArgumentException("bad argument with 'defstruct( structname, definition )'");

    const char* structname = luaL_checkstring(L, 1);
    if (!validate_typename(structname))
        throw AlienInvalidArgumentException("bad struct name '%s'", structname);

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
         throw AlienInvalidArgumentException("can't define type '%s' twice", structname);
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
            throw AlienInvalidValueException("defstruct bad member definition");

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

    alien_push_raw_type_table(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
    ALIEN_EXCEPTION_END();
}

/** similar with struct, see above */
ALIEN_LUA_FUNC int alien_types_defunion(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_istable(L, 2))
        throw AlienInvalidArgumentException("bad argument with 'defunion( unionname, definition )'");

    const char* unionname = luaL_checkstring(L, 1);
    if (!validate_typename(unionname))
        throw AlienInvalidArgumentException("bad struct name '%s'", unionname);

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1))
        throw AlienInvalidArgumentException("can't define type '%s' twice", unionname);
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
            throw AlienInvalidValueException("defunion bad member definition");

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

    alien_push_raw_type_table(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    return 1;
    ALIEN_EXCEPTION_END();
}

/** alias(newtypename, type) */
ALIEN_LUA_FUNC int alien_types_alias(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    const char* nname = luaL_checkstring(L, 1);
    if (!alien_istype(L, 2))
        throw AlienInvalidArgumentException("bad argument with 'alias(newtypename, type)'");

    if (!validate_typename(nname))
        throw AlienInvalidArgumentException("bad type name '%s'", nname);

    alien_push_raw_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1))
        throw AlienInvalidArgumentException("can't set alias '%s', type '%s' has existed", nname, nname);
    lua_pop(L, 1);

    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    return 0;
    ALIEN_EXCEPTION_END();
}

/** atype(typename) */
ALIEN_LUA_FUNC int alien_types_getbyname(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    const char* nname = luaL_checkstring(L, 1);
    auto enret = ensure_type(L, nname);

    if (enret.first == nullptr)
        throw AlienException("%s", enret.second.c_str());

    alien_push_type_table(L);
    lua_pushvalue(L, 1);
    lua_gettable(L, -2);
    assert(!lua_isnil(L, -1));
    return 1;
    ALIEN_EXCEPTION_END();
}

