#include "alien_type_pointer.h"
#include "alien_value_pointer.h"
#include <ffi.h>


alien_type_pointer::alien_type_pointer(alien_type* _ptr_type):
    alien_type(_ptr_type->__typename() + "*"), _ptr_type(_ptr_type) {}

ffi_type* alien_type_pointer::ffitype() {
    return nullptr;
}

alien_value* alien_type_pointer::from_lua(lua_State* L, int idx) const {
    return alien_value_pointer::from_lua(this, L, idx);
}

alien_value* alien_type_pointer::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_pointer::from_ptr(this, L, ptr);
}

alien_value* alien_type_pointer::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_pointer::from_shr(this, L, m, ptr);
}

alien_value* alien_type_pointer::new_value(lua_State* L) const {
    return alien_value_pointer::new_value(this, L);
}

const alien_type* alien_type_pointer::ptr_type() const {
    return this->_ptr_type;
}

alien_type* alien_type_pointer::ptr_type() {
    return this->_ptr_type;
}

bool alien_type_pointer::is_pointer() const { return true; }

bool alien_type_pointer::is_this_type(lua_State* L, int idx) const {
    return alien_value_pointer::is_this_value(this, L, idx);
}

alien_value* alien_type_pointer::checkvalue(lua_State* L, int idx) const {
    return alien_value_pointer::checkvalue(this, L, idx);
}

