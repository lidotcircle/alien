#include "alien_value.h"
#include "alien_value_basic.h"
#include "alien_value_buffer.h"
#include "alien_value_callback.h"
#include "alien_value_pointer.h"
#include "alien_value_ref.h"
#include "alien_value_string.h"
#include "alien_value_struct.h"
#include "alien_value_union.h"
#include <memory>
#include <stdexcept>
#include <string.h>
#include <assert.h>
using namespace std;

/** operator dispatcher */
int alien_operator_method_new(lua_State* L, alien_type* type) {
    if (type->is_basic()) {
        return alien_value_basic_new(L);
    } else if (type->is_buffer()) {
        return alien_value_buffer_new(L);
    } else if (type->is_callback()) {
        return alien_value_callback_new(L);
    } else if (type->is_pointer()) {
        return alien_value_pointer_new(L);
    } else if (type->is_ref()) {
        return alien_value_ref_new(L);
    } else if (type->is_string()) {
        return alien_value_string_new(L);
    } else if (type->is_struct()) {
        return alien_value_struct_new(L);
    } else if (type->is_union()) {
        return alien_value_union_new(L);
    } else {
        return luaL_error(L, "alien: unsupported type");
    }

    return 1;
}

#define MAX_ab(a, b) ((a) > (b) ? (a) : (b))

alien_value::alien_value(const alien_type* type):
    type(type),
    _mem(type->__sizeof() > 0 
            ? (new char[MAX_ab(type->__sizeof(), sizeof(void*))]) 
            : nullptr, 
        [](char ptr[]) { if (ptr != nullptr) delete[] ptr;}),
    val_ptr(_mem.get()) {}

alien_value::alien_value(const alien_type* type, std::shared_ptr<char> mem, void* ptr):
    type(type),
    _mem(mem),
    val_ptr(ptr) {}

void* alien_value::ptr() { return this->val_ptr; }
const void* alien_value::ptr() const { return this->val_ptr; }
size_t alien_value::__sizeof() const { return this->type->__sizeof(); }
const alien_type* alien_value::alientype() const { return this->type; }

void alien_value::assignFrom(const alien_value& val) {
    if (this->type != val.type)
        throw std::runtime_error("alien: assignment between incompatible type");

    memcpy(this->ptr(), const_cast<void*>(val.ptr()), this->__sizeof());
}

void alien_value::assignFromLua(lua_State* L, size_t idx) {
    auto v = this->type->from_lua(L, idx);
    this->assignFrom(*v);
    delete v;
}


#include "alien_value_basic.h"
#include "alien_value_buffer.h"
#include "alien_value_callback.h"
#include "alien_value_pointer.h"
#include "alien_value_ref.h"
#include "alien_value_string.h"
#include "alien_value_struct.h"
#include "alien_value_union.h"
int alien_value_init(lua_State* L) {
    auto n = lua_gettop(L);

    alien_value_basic_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_buffer_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_callback_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_pointer_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_ref_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_string_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_struct_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    alien_value_union_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    return 0;
}

