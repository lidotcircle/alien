#include "alien_type_string.h"
#include "alien_value_string.h"

alien_type_string::alien_type_string(): alien_type("string") {}

ffi_type* alien_type_string::ffitype() {
    return nullptr;
}

alien_value* alien_type_string::from_lua(lua_State* L, int idx) const {
    return alien_value_string::from_lua(this, L, idx);
}

alien_value* alien_type_string::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_string::from_ptr(this, L, ptr);
}

alien_value* alien_type_string::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_string::from_shr(this, L, m, ptr);
}

alien_value* alien_type_string::new_value(lua_State* L) const {
    return alien_value_string::new_value(this, L);
}

bool alien_type_string::is_string() const { return true; }

bool alien_type_string::is_this_type(lua_State* L, int idx) const {
    return alien_value_string::is_this_value(this, L, idx);
}

alien_value* alien_type_string::checkvalue(lua_State* L, int idx) const {
    return alien_value_string::checkvalue(this, L, idx);
}

