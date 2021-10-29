#include "alien_type_pointer.h"
#include "alien_value_pointer.h"
#include <ffi.h>

static ffi_type ffi_type_alien_pointer = ffi_type_pointer;


alien_type_pointer::alien_type_pointer(): alien_type("pointer") {}

ffi_type* alien_type_pointer::ffitype() {
    return &ffi_type_alien_pointer;
}

alien_value* alien_type_pointer::from_lua(lua_State* L, int idx) const {
    return alien_value_pointer::from_lua(this, L, idx);
}

alien_value* alien_type_pointer::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_pointer::from_ptr(this, L, ptr);
}

alien_value* alien_type_pointer::new_value(lua_State* L) const {
    return alien_value_pointer::new_value(this, L);
}

bool alien_type_pointer::is_pointer() const { return true; }

