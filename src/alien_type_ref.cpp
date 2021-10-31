#include "alien_type_ref.h"
#include "alien_value_ref.h"
#include <ffi.h>

static ffi_type ffi_type_ref = ffi_type_pointer;


alien_type_ref::alien_type_ref(alien_type* reftype): alien_type("ref"), _ref_type(reftype) {}

ffi_type* alien_type_ref::ffitype() {
    return &ffi_type_ref;
}

alien_value* alien_type_ref::from_lua(lua_State* L, int idx) const {
    return alien_value_ref::from_lua(this, L, idx);
}

alien_value* alien_type_ref::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_ref::from_ptr(this, L, ptr);
}

alien_value* alien_type_ref::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_ref::from_shr(this, L, m, ptr);
}

alien_value* alien_type_ref::new_value(lua_State* L) const {
    return alien_value_ref::new_value(this, L);
}

const alien_type* alien_type_ref::ref_type() const {
    return this->_ref_type;
}

alien_type* alien_type_ref::ref_type() {
    return this->_ref_type;
}

bool alien_type_ref::is_ref() const { return true; }

bool alien_type_ref::is_this_type(lua_State* L, int idx) const {
    return alien_value_ref::is_this_value(this, L, idx);
}

alien_value* alien_type_ref::checkvalue(lua_State* L, int idx) const {
    return alien_value_ref::checkvalue(this, L, idx);
}

