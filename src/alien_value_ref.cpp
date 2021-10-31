#include "alien_value_ref.h"
#include "alien_type_ref.h"
#include "alien_value_struct.h"
#include "alien_value_union.h"
#include <assert.h>
#include <string.h>
using namespace std;

#define ALIEN_VALUE_REF_META "alien_value_ref_meta"

static bool alien_isref(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_VALUE_REF_META) != nullptr;
}
static alien_value_ref* alien_checkref(lua_State* L, int idx) {
    return *static_cast<alien_value_ref**>(luaL_checkudata(L, idx, ALIEN_VALUE_REF_META));
}

static int alien_value_ref_gc(lua_State* L);
static int alien_value_ref_tostring(lua_State* L);
static int alien_value_ref_get_member(lua_State* L);
static int alien_value_ref_set_member(lua_State* L);

int alien_value_ref_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_VALUE_REF_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_ref_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_ref_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_value_ref_get_member);
    lua_settable(L, -3);
    lua_pushliteral(L, "__setindex");
    lua_pushcfunction(L, alien_value_ref_set_member);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

int alien_value_ref_new(lua_State* L) {
    alien_type* reftype = alien_checktype(L, 1);
    std::unique_ptr<alien_value> val(reftype->from_lua(L, 1));
    val->to_lua(L);

    return 1;
}

static int alien_value_ref_gc(lua_State* L) {
    alien_value_ref* ref = alien_checkref(L, 1);
    delete ref;
    return 0;
}
static int alien_value_ref_tostring(lua_State* L) {
    alien_value_ref* ref = alien_checkref(L, 1);
    lua_pushfstring(L, "alien ref 0x%x", ref->ptr());
    return 1;
}
static int alien_value_ref_get_member(lua_State* L) {
    alien_value_ref* ref = alien_checkref(L, 1);
    const char* name = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> mem(ref->access_member(L, name));
    if (mem == nullptr)
        return luaL_error(L, "alien: get member named '%s' failed", name);
    mem->to_lua(L);
    return 1;
}
static int alien_value_ref_set_member(lua_State* L) {
    alien_value_ref* ref = alien_checkref(L, 1);
    const char* name = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> member(ref->access_member(L, name));
    member->assignFromLua(L, 3);
    return 0;
}

alien_value_ref::alien_value_ref(const alien_type* type): 
    alien_value(type), pref_info(std::make_shared<ref_info>())
{
    assert(type->is_ref());
    auto rinfo = this->pref_info;
    const alien_type_ref* uv = dynamic_cast<const alien_type_ref*>(this->alientype());
    assert(uv != nullptr);
    rinfo->ref_type = const_cast<alien_type*>(uv->ref_type());

    rinfo->__uptr = std::shared_ptr<char>(new char[rinfo->ref_type->__sizeof()], std::default_delete<char[]>());
    memset(rinfo->__uptr.get(), 0, rinfo->ref_type->__sizeof());
    rinfo->ref_ptr = rinfo->__uptr.get();

    *static_cast<void**>(this->ptr()) = rinfo->ref_ptr;
    this->init_value();
}

alien_value_ref::alien_value_ref(const alien_type* type, std::shared_ptr<char> mem, void* ptr):
    alien_value(type), pref_info(std::make_shared<ref_info>())
{
    assert(type->is_ref());
    auto rinfo = this->pref_info;
    const alien_type_ref* uv = dynamic_cast<const alien_type_ref*>(this->alientype());
    assert(uv != nullptr);
    rinfo->ref_type = const_cast<alien_type*>(uv->ref_type());

    rinfo->__uptr = mem;
    rinfo->ref_ptr = ptr;

    *static_cast<void**>(this->ptr()) = rinfo->ref_ptr;
    this->init_value();
}

alien_value_ref::alien_value_ref(const alien_value_ref& other): alien_value(other), pref_info(other.pref_info) 
{
}

void alien_value_ref::init_value() {
    if (this->pref_info->aval != nullptr) return;

    void* ptr = *static_cast<void**>(this->ptr());
    // TODO
    this->pref_info->aval = this->pref_info->ref_type->from_ptr(nullptr, ptr);
}

void alien_value_ref::to_lua(lua_State* L) const {
    alien_value_ref** pvp = static_cast<alien_value_ref**>(lua_newuserdata(L, sizeof(alien_value_ref*)));
    *pvp = dynamic_cast<alien_value_ref*>(this->copy());
    luaL_setmetatable(L, ALIEN_VALUE_REF_META);
}

alien_value* alien_value_ref::copy() const {
    void* ptr = *static_cast<void* const*>(this->ptr());
    alien_value_ref* ans = new alien_value_ref(this->alientype(), this->_mem, ptr);
    return ans;
}

alien_value* alien_value_ref::access_member(lua_State* L, const string& m) const {
    auto rinfo = this->pref_info;
    if (rinfo->ref_type->is_struct()) {
        alien_value_struct* sval = dynamic_cast<alien_value_struct*>(rinfo->aval);
        if (sval == nullptr)
            throw std::runtime_error("alien: bad value of struct type");
        auto ans = sval->get_member(m);

        if (ans == nullptr) {
            luaL_error(L, "alien: access struct member with ref failed");
            return nullptr;
        }

        return ans;
    } else if (rinfo->ref_type->is_union()) {
        alien_value_union* uval = dynamic_cast<alien_value_union*>(rinfo->aval);
        if (uval == nullptr)
            throw std::runtime_error("alien: bad value of union type");
        auto ans = uval->get_member(m);

        if (ans == nullptr) {
            luaL_error(L, "alien: access union member with ref failed");
            return nullptr;
        }

        return ans;
    } else {
        luaL_error(L, "alien: access member of non struct or union object");
        return nullptr;
    }
}

alien_value_ref::~alien_value_ref() {}
alien_value_ref::ref_info::~ref_info() {
    delete this->aval;
}

/** static */
alien_value* alien_value_ref::from_lua(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_ref());
    const alien_type_ref* uv = dynamic_cast<const alien_type_ref*>(type);
    auto reft = uv->ref_type();

    if (lua_isnil(L, idx)) {
        return new alien_value_ref(type);
    } else if (reft->is_this_type(L, idx)) {
        alien_value* v = reft->checkvalue(L, idx);
        auto ans = new alien_value_ref(type, v->_mem, v->ptr());
        if (reft->is_basic() || reft->is_string())
            delete v;
        return ans;
    } else if (alien_isref(L, idx)) {
        alien_value_ref* ptr = alien_checkref(L, idx);
        return ptr->copy();
    } else {
        luaL_error(L, "alien: bad argument to alien_value_ref");
        return nullptr;
    }
}

/** static */
alien_value* alien_value_ref::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    // TODO
    return new alien_value_ref(type, std::make_shared<char>(), ptr);
}

/** static */
alien_value* alien_value_ref::new_value(const alien_type* type, lua_State* L) {
    return new alien_value_ref(type);
}

/** static */
bool alien_value_ref::is_this_value(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_ref());
    return alien_isref(L, idx);
}

/** static */
alien_value_ref* alien_value_ref::checkvalue(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_ref());
    return alien_checkref(L, idx);
}

