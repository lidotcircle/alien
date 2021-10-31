#include "alien_type_basic.h"
#include "alien_value_basic.h"
#include <ffi.h>
#include <set>

#define MAX_ab(a, b) ((a) > (b) ? (a) : (b))


alien_type_basic::alien_type_basic(const std::string& t, ffi_abi abi, ffi_type* type):
    alien_type(t), abi(abi), pffi_type(type) {}

ffi_type* alien_type_basic::ffitype() { return this->pffi_type; }

static const std::set<ffi_type*> integer_types = {
    &ffi_type_uint8,  &ffi_type_sint8,
    &ffi_type_uint16, &ffi_type_sint16,
    &ffi_type_uint32, &ffi_type_sint32,
    &ffi_type_uint64, &ffi_type_sint64,
    &ffi_type_uchar,  &ffi_type_schar,
    &ffi_type_ushort, &ffi_type_sshort,
    &ffi_type_uint,   &ffi_type_sint,
    &ffi_type_ulong,  &ffi_type_slong,
};
static const std::set<ffi_type*> signed_integer_types = {
    &ffi_type_sint8,
    &ffi_type_sint16,
    &ffi_type_sint32,
    &ffi_type_sint64,
    &ffi_type_schar,
    &ffi_type_sshort,
    &ffi_type_sint,
    &ffi_type_slong,
};

alien_value* alien_type_basic::from_lua(lua_State* L, int idx) const {
    return alien_value_basic::from_lua(this, L, idx);
}

alien_value* alien_type_basic::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_basic::from_ptr(this, L, ptr);
}

alien_value* alien_type_basic::from_shr(lua_State* L, std::shared_ptr<char> mem, void* ptr) const {
    return alien_value_basic::from_shr(this, L, mem, ptr);
}

alien_value* alien_type_basic::new_value(lua_State* L) const {
    return alien_value_basic::new_value(this, L);
}

bool alien_type_basic::is_integer() const {
    return integer_types.find(const_cast<alien_type_basic*>(this)->ffitype()) != integer_types.end();
}

bool alien_type_basic::is_signed()  const {
    return signed_integer_types.find(const_cast<alien_type_basic*>(this)->ffitype()) != signed_integer_types.end();
}

bool alien_type_basic::is_float() const {
    return const_cast<alien_type_basic*>(this)->ffitype() == &ffi_type_float;
}

bool alien_type_basic::is_double() const {
    return const_cast<alien_type_basic*>(this)->ffitype() == &ffi_type_double;
}

bool alien_type_basic::is_rawpointer() const {
    return const_cast<alien_type_basic*>(this)->ffitype() == &ffi_type_pointer;
}

bool alien_type_basic::is_void() const {
    return const_cast<alien_type_basic*>(this)->ffitype() == &ffi_type_void;
}

bool alien_type_basic::is_basic() const { return true; }

alien_type_basic::~alien_type_basic() { }

bool alien_type_basic::is_this_type(lua_State* L, int idx) const {
    return alien_value_basic::is_this_value(this, L, idx);
}

alien_value* alien_type_basic::checkvalue(lua_State* L, int idx) const {
    return alien_value_basic::checkvalue(this, L, idx);
}

