#include "alien_type_buffer.h"

ffi_type ffi_type_buffer = ffi_type_pointer;


alien_type_buffer::alien_type_buffer(): alien_type("buffer") {}

ffi_type* alien_type_buffer::ffitype() { return &ffi_type_buffer; }

size_t alien_type_buffer::__sizeof() const { return sizeof(void*); }

alien_value* alien_type_buffer::fromLua(lua_State* L, int idx) const {
    return nullptr;
}

alien_value* alien_type_buffer::fromPtr(lua_State* L, void* ptr) const {
    return nullptr;
}

alien_value* alien_type_buffer::new_value() const {
}

bool alien_type_buffer::is_buffer() const { return true; }

alien_type_buffer::~alien_type_buffer() {}

