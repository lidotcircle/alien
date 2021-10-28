#include "alien_type_callback.h"

static ffi_type ffi_callback = ffi_type_pointer;


alien_type_callback::alien_type_callback():
    alien_type("callback") {}

ffi_type* alien_type_callback::ffitype() {
    return &ffi_callback;
}

alien_value* alien_type_callback::fromLua(lua_State* L, int idx) const {
}

alien_value* alien_type_callback::new_value() const {
    return nullptr;
}

bool alien_type_callback::is_callback() const { return true; }

