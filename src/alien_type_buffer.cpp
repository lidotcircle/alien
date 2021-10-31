#include "alien_type_buffer.h"
#include "alien_value_buffer.h"

ffi_type ffi_type_buffer = ffi_type_pointer;


alien_type_buffer::alien_type_buffer(): alien_type("buffer") {}

ffi_type* alien_type_buffer::ffitype() { return &ffi_type_buffer; }

size_t alien_type_buffer::__sizeof() const { return sizeof(void*); }

alien_value* alien_type_buffer::from_lua(lua_State* L, int idx) const {
    return alien_value_buffer::from_lua(this, L, idx);
}

alien_value* alien_type_buffer::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_buffer::from_ptr(this, L, ptr);
}

alien_value* alien_type_buffer::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_buffer::from_shr(this, L, m, ptr);
}

alien_value* alien_type_buffer::new_value(lua_State* L) const {
    return alien_value_buffer::new_value(this, L);
}

bool alien_type_buffer::is_buffer() const { return true; }

alien_type_buffer::~alien_type_buffer() {}

bool alien_type_buffer::is_this_type(lua_State* L, int idx) const {
    return alien_value_buffer::is_this_value(this, L, idx);
}

alien_value* alien_type_buffer::checkvalue(lua_State* L, int idx) const {
    return alien_value_buffer::checkvalue(this, L, idx);
}

