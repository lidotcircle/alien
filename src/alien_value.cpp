#include "alien_value.h"
#include "alien_value_basic.h"
#include "alien_value_buffer.h"
#include "alien_value_callback.h"
#include "alien_value_pointer.h"
#include "alien_value_ref.h"
#include "alien_value_string.h"
#include "alien_value_struct.h"
#include "alien_value_union.h"
#include "alien_value_array.h"
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
    } else if (type->is_array()) {
        return alien_value_array_new(L);
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
#include "alien_value_array.h"
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

    alien_value_array_init(L);
    assert(n == lua_gettop(L) && lua_istable(L, -1));

    return 0;
}

int alien_value_copy(lua_State* L) {
    if (lua_gettop(L) != 1)
        return luaL_error(L, "alien: copy() require exactly 1 argument");

    auto type = lua_type(L, 1);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
        case LUA_TBOOLEAN:
        case LUA_TNUMBER:
        case LUA_TSTRING:
        case LUA_TFUNCTION:
        case LUA_TLIGHTUSERDATA:
            return 1;
        case LUA_TTHREAD:
            return luaL_error(L, "alien: can't copy thread");
        case LUA_TTABLE:
            return luaL_error(L, "alien: can't copy table");
        case LUA_TUSERDATA:
            break;
    }

    // TODO
    alien_value* val = nullptr;
    if (alien_value_buffer::is_this_value(nullptr, L, 1))
        val = alien_value_buffer::checkvalue(nullptr, L, 1);
    else if (alien_value_pointer::is_this_value(nullptr, L, 1))
        val = alien_value_pointer::checkvalue(nullptr, L, 1);
    else if (alien_value_ref::is_this_value(nullptr, L, 1))
        val = alien_value_ref::checkvalue(nullptr, L, 1);
    else if (alien_value_struct::is_this_value(nullptr, L, 1))
        val = alien_value_struct::checkvalue(nullptr, L, 1);
    else if (alien_value_union::is_this_value(nullptr, L, 1))
        val = alien_value_union::checkvalue(nullptr, L, 1);

    if (val == nullptr)
        return luaL_error(L, "alien: can't copy alien value");

    std::unique_ptr<alien_value> vv(val->copy());
    vv->to_lua(L);
    return 1;
}

alien_value* alien_generic_from_lua(lua_State* L, int idx) {
    auto type = lua_type(L, idx);
    auto str_type = alien_type_byname(L, "string");
    auto rawptr_t = alien_type_byname(L, "rawpointer");
    switch (type) {
        case LUA_TNIL:
        {
            lua_pushnumber(L, 0);
            auto ans = rawptr_t->from_lua(L, -1);
            lua_remove(L, -1);
            return ans; 
        }
        case LUA_TSTRING:
            return str_type->from_lua(L, idx);
        case LUA_TLIGHTUSERDATA:
            return rawptr_t->from_lua(L, -1);
        case LUA_TNONE:
            luaL_error(L, "alien: can't checkvalue with none type"); break;
        case LUA_TBOOLEAN:
            luaL_error(L, "alien: can't checkvalue with boolean type"); break;
        case LUA_TNUMBER:
            luaL_error(L, "alien: can't checkvalue with number type"); break;
        case LUA_TFUNCTION:
            luaL_error(L, "alien: can't checkvalue with function type"); break;
        case LUA_TTHREAD:
            luaL_error(L, "alien: can't checkvalue with thread type"); break;
        case LUA_TTABLE:
            luaL_error(L, "alien: can't checkvalue with table type"); break;
        case LUA_TUSERDATA:
            break;
    }

    auto empty_delete = [](alien_value*) {};
    shared_ptr<alien_value> val;
    if (alien_value_buffer::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_buffer::checkvalue(nullptr, L, idx),
                empty_delete);
    else if (alien_value_pointer::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_pointer::checkvalue(nullptr, L, idx),
                empty_delete);
    else if (alien_value_ref::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_ref::checkvalue(nullptr, L, idx),
                empty_delete);
    else if (alien_value_struct::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_struct::checkvalue(nullptr, L, idx),
                empty_delete);
    else if (alien_value_union::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_union::checkvalue(nullptr, L, idx),
                empty_delete);
    else if (alien_value_basic::is_this_value(nullptr, L, idx))
        val = shared_ptr<alien_value>(
                alien_value_basic::checkvalue(nullptr, L, idx),
                std::default_delete<alien_value>());

    if (val == nullptr) {
        luaL_error(L, "alien: can't get alien value");
        return nullptr;
    }
    return val->copy();
}

