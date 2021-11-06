#include "alien_type_callback.h"
#include "alien_value_callback.h"
#include <stdexcept>


alien_type_callback::alien_type_callback():
    alien_type("callback") {}

ffi_type* alien_type_callback::ffitype() {
    return nullptr;
}

alien_value* alien_type_callback::from_lua(lua_State* L, int idx) const {
    return alien_value_callback::from_lua(this, L, idx);
}

alien_value* alien_type_callback::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_callback::from_ptr(this, L, ptr);
}

alien_value* alien_type_callback::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_callback::from_shr(this, L, m, ptr);
}

alien_value* alien_type_callback::new_value(lua_State* L) const {
    return alien_value_callback::new_value(this, L);
}

bool alien_type_callback::is_callback() const { return true; }

bool alien_type_callback::is_this_type(lua_State* L, int idx) const {
    return alien_value_callback::is_this_value(this, L, idx);
}

alien_value* alien_type_callback::checkvalue(lua_State* L, int idx) const {
    return alien_value_callback::checkvalue(this, L, idx);
}

