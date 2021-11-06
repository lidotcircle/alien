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

alien_type* alien_checktype(lua_State* L, int idx)
{
    return nullptr;
}

int alien_push_type_table(lua_State* L) {
    return 0;
}

alien_type* alien_type_byname(lua_State* L, const char* name)
{
    return nullptr;
}

alien_type* alien_reftype(lua_State* L, alien_type* type) {
    if (type->is_ref())
        throw AlienException("type is already a reference");

    return nullptr;
}

alien_type* alien_ptrtype(lua_State* L, alien_type* type) {
    return nullptr;
}


int alien_types_init(lua_State* L)     { return 0;
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
ALIEN_LUA_FUNC int alien_types_defstruct(lua_State* L) 
{ return 0; }

/** similar with struct, see above */
ALIEN_LUA_FUNC int alien_types_defunion(lua_State* L) 
{return 0;}

/** alias(newtypename, type) */
ALIEN_LUA_FUNC int alien_types_alias(lua_State* L) 
{ return 0;}

/** atype(typename) */
ALIEN_LUA_FUNC int alien_types_getbyname(lua_State* L) 
{return 0;}

