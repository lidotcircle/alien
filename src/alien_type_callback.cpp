#include "alien_type_callback.h"
#include "alien_value_callback.h"
#include <stdexcept>

static ffi_type ffi_callback = ffi_type_pointer;


alien_type_callback::alien_type_callback():
    alien_type("callback") {}

ffi_type* alien_type_callback::ffitype() {
    return &ffi_callback;
}

alien_value* alien_type_callback::from_lua(lua_State* L, int idx) const {
    return alien_value_callback::from_lua(this, L, idx);
}

alien_value* alien_type_callback::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_callback::from_ptr(this, L, ptr);
}

alien_value* alien_type_callback::new_value(lua_State* L) const {
    return alien_value_callback::new_value(this, L);
}

bool alien_type_callback::is_callback() const { return true; }

