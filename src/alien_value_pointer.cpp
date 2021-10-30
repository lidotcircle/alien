#include "alien_value_pointer.h"
#include "alien_type_pointer.h"
#include "alien_value_struct.h"
#include "alien_value_union.h"
#include <assert.h>
using namespace std;

#define ALIEN_VALUE_POINTER_META "alien_value_pointer_meta"

static bool alien_ispointer(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_VALUE_POINTER_META) != nullptr;
}
static alien_value_pointer* alien_checkpointer(lua_State* L, int idx) {
    return *static_cast<alien_value_pointer**>(luaL_checkudata(L, idx, ALIEN_VALUE_POINTER_META));
}

static int alien_value_pointer_gc(lua_State* L);
static int alien_value_pointer_tostring(lua_State* L);
static int alien_value_pointer_get_member(lua_State* L);
static int alien_value_pointer_set_member(lua_State* L);
static int alien_value_pointer_deref(lua_State* L);
static int alien_value_pointer_raw(lua_State* L);

int alien_value_pointer_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_VALUE_POINTER_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_pointer_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_pointer_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_newtable(L);
    lua_pushliteral(L, "get_member");
    lua_pushcfunction(L, alien_value_pointer_get_member);
    lua_settable(L, -3);
    lua_pushliteral(L, "set_member");
    lua_pushcfunction(L, alien_value_pointer_set_member);
    lua_settable(L, -3);
    lua_pushliteral(L, "deref");
    lua_pushcfunction(L, alien_value_pointer_deref);
    lua_settable(L, -3);
    lua_pushliteral(L, "raw");
    lua_pushcfunction(L, alien_value_pointer_raw);
    lua_settable(L, -3);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

static int alien_value_pointer_gc(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    delete pointer;
    return 0;
}
static int alien_value_pointer_tostring(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    lua_pushfstring(L, "alien pointer 0x%x", pointer->ptr());
    return 1;
}
static int alien_value_pointer_get_member(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    const char* name = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> mem(pointer->access_member(L, name));
    if (mem == nullptr)
        return luaL_error(L, "alien: get member named '%s' failed", name);
    mem->to_lua(L);
    return 1;
}
static int alien_value_pointer_set_member(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    const char* name = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> member(pointer->access_member(L, name));
    member->assignFromLua(L, 3);
    return 0;
}
static int alien_value_pointer_deref(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    std::unique_ptr<alien_value> val(pointer->deref());
    if (val == nullptr)
        return luaL_error(L, "alien: deref failed");
    val->to_lua(L);
    return 1;
}
static int alien_value_pointer_raw(lua_State* L) {
    alien_value_pointer* pointer = alien_checkpointer(L, 1);
    lua_pushlightuserdata(L, pointer->ptr());
    return 1;
}

alien_value_pointer::alien_value_pointer(const alien_type* type): alien_value(type), aval(nullptr)
{
    assert(type->is_pointer());
    *static_cast<void**>(this->ptr()) = nullptr;
    const alien_type_pointer* uv = dynamic_cast<const alien_type_pointer*>(this->alientype());
    assert(uv != nullptr);
    this->ptr_type = const_cast<alien_type*>(uv->ptr_type());
    this->init_value();
}

alien_value_pointer::alien_value_pointer(const alien_type* type, void* ptr): alien_value(type), aval(nullptr)
{
    assert(type->is_pointer());
    *static_cast<void**>(this->ptr()) = ptr;
    const alien_type_pointer* uv = dynamic_cast<const alien_type_pointer*>(this->alientype());
    assert(uv != nullptr);
    this->ptr_type = const_cast<alien_type*>(uv->ptr_type());
    this->init_value();
}

void alien_value_pointer::init_value() {
    if (aval != nullptr) return;

    void* ptr = *static_cast<void**>(this->ptr());
    // TODO
    aval = this->ptr_type->from_ptr(nullptr, ptr);
}

void alien_value_pointer::to_lua(lua_State* L) const {
    alien_value_pointer** pvp = static_cast<alien_value_pointer**>(lua_newuserdata(L, sizeof(alien_value_pointer*)));
    *pvp = dynamic_cast<alien_value_pointer*>(this->copy());
    luaL_setmetatable(L, ALIEN_VALUE_POINTER_META);
}

alien_value* alien_value_pointer::copy() const {
    void* ptr = *static_cast<void* const*>(this->ptr());
    alien_value_pointer* ans = new alien_value_pointer(this->alientype(), ptr);
    return ans;
}

alien_value* alien_value_pointer::access_member(lua_State* L, const string& m) const {
    if (this->ptr_type->is_struct()) {
        alien_value_struct* sval = dynamic_cast<alien_value_struct*>(this->aval);
        if (sval == nullptr)
            throw std::runtime_error("alien: bad value of struct type");
        auto ans = sval->get_member(m);

        if (ans == nullptr) {
            luaL_error(L, "alien: access struct member with pointer failed");
            return nullptr;
        }

        return ans;
    } else if (this->ptr_type->is_union()) {
        alien_value_union* uval = dynamic_cast<alien_value_union*>(this->aval);
        if (uval == nullptr)
            throw std::runtime_error("alien: bad value of union type");
        auto ans = uval->get_member(m);

        if (ans == nullptr) {
            luaL_error(L, "alien: access union member with pointer failed");
            return nullptr;
        }

        return ans;
    } else {
        luaL_error(L, "alien: access member of non struct or union object");
        return nullptr;
    }
}

alien_value* alien_value_pointer::deref() const {
    return this->aval->copy();
}

bool alien_value_pointer::is_null() const {
    return *static_cast<void*const*>(this->ptr()) == nullptr;
}

alien_value_pointer::~alien_value_pointer() {
    if (aval != nullptr) {
        delete aval;
        aval = nullptr;
    }
}

/** static */
alien_value* alien_value_pointer::from_lua(const alien_type* type, lua_State* L, int idx) {
    if (lua_isnil(L, idx)) {
        return new alien_value_pointer(type, nullptr);
    } else if (lua_islightuserdata(L, idx)) {
        void* ptr = lua_touserdata(L, idx);
        return new alien_value_pointer(type, ptr);
    } else if (alien_ispointer(L, idx)) {
        alien_value_pointer* ptr = alien_checkpointer(L, idx);
        return ptr->copy();
    } else {
        luaL_error(L, "alien: bad argument to alien_value_pointer");
        return nullptr;
    }
}

/** static */
alien_value* alien_value_pointer::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    return new alien_value_pointer(type, ptr);
}

/** static */
alien_value* alien_value_pointer::new_value(const alien_type* type, lua_State* L) {
    return new alien_value_pointer(type);
}

/** static */
bool alien_value_pointer::is_this_value(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_pointer());
    return alien_ispointer(L, idx);
}

/** static */
alien_value_pointer* alien_value_pointer::checkvalue(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_pointer());
    return alien_checkpointer(L, idx);
}

