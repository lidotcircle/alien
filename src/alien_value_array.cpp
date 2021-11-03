#include "alien_value_array.h"
#include "alien_type_array.h"
#include <string>
#include <assert.h>
#include <string.h>
using namespace std;


#define ALIEN_VALUE_ARRAY_META "alien_value_array_meta"


static bool alien_isarray(lua_State* L, int idx)
{
    return luaL_testudata(L, idx, ALIEN_VALUE_ARRAY_META) != nullptr;
}

static alien_value_array* alien_checkarray(lua_State* L, int idx)
{
    return *static_cast<alien_value_array**>(luaL_checkudata(L, idx, ALIEN_VALUE_ARRAY_META));
}

static int alien_value_array_gc(lua_State* L);
static int alien_value_array_tostring(lua_State* L);
static int alien_value_array_index(lua_State* L);
static int alien_value_array_newindex(lua_State* L);

int alien_value_array_init(lua_State* L)
{
    luaL_newmetatable(L, ALIEN_VALUE_ARRAY_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_array_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_array_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_value_array_index);
    lua_settable(L, -3);

    lua_pushliteral(L, "__newindex");
    lua_pushcfunction(L, alien_value_array_newindex);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

int alien_value_array_new(lua_State* L) {
    alien_type* arraytype = alien_checktype(L, 1);
    std::unique_ptr<alien_value> val(arraytype->new_value(L));
    val->to_lua(L);

    return 1;
}

static int alien_value_array_gc(lua_State* L)
{
    alien_value_array* s = alien_checkarray(L, 1);
    delete s;
    return 0;
}
static int alien_value_array_tostring(lua_State* L)
{
    alien_value_array* s = alien_checkarray(L, 1);
    lua_pushfstring(L, "[alien array: %p]", s);
    return 1;
}
static int alien_value_array_index(lua_State* L)
{
    alien_value_array* s = alien_checkarray(L, 1);
    int n = luaL_checkinteger(L, 2);
    if (n >s->length() || n <= 0)
        return luaL_error(L, "alien: array index out of range");

    std::unique_ptr<alien_value> mem(s->get_n(n));
    if (mem == nullptr)
        return luaL_error(L, "alien: array fatal error");

    mem->to_lua(L);
    return 1;
}
static int alien_value_array_newindex(lua_State* L)
{
    alien_value_array* s = alien_checkarray(L, 1);
    int n = luaL_checkinteger(L, 2);
    if (n >s->length() || n <= 0)
        return luaL_error(L, "alien: array index out of range");

    std::unique_ptr<alien_value> mb(s->get_n(n));
    if (mb == nullptr)
        return luaL_error(L, "alien: array fatal error");

    mb->assignFromLua(L, 3);
    return 0;
}

alien_value_array::alien_value_array(const alien_type* type): alien_value(type)
{
}

alien_value_array::alien_value_array(const alien_type* type, std::shared_ptr<char> mem, void* ptr):
    alien_value(type, mem, ptr)
{
}

alien_value* alien_value_array::get_n(size_t n) 
{
    const alien_type_array* t = dynamic_cast<const alien_type_array*>(this->alientype());
    if (t == nullptr)
        return nullptr;

    assert(n > 0 && n <= this->length());
    auto mt = dynamic_cast<const alien_type_array*>(this->alientype());
    auto et = mt->elemtype();
    void* ptr = static_cast<char*>(this->ptr()) + (n - 1) * mt->element_aligned_size();

    // TODO
    return et->from_shr(nullptr, this->_mem, ptr);
}

const alien_value* alien_value_array::get_n(size_t n) const
{
    return const_cast<alien_value_array*>(this)->get_n(n);
}

size_t alien_value_array::length() const {
    auto at = dynamic_cast<const alien_type_array*>(this->alientype());
    assert(at != nullptr);
    return at->array_length();
}

void alien_value_array::to_lua(lua_State* L) const
{
    alien_value_array* obj = 
        new alien_value_array(this->alientype(), this->_mem, const_cast<void*>(this->ptr()));
    alien_value_array** p = static_cast<alien_value_array**>(lua_newuserdata(L, sizeof(alien_value_array*)));
    *p = obj;
    luaL_setmetatable(L, ALIEN_VALUE_ARRAY_META);
}

alien_value* alien_value_array::copy() const {
    alien_value_array* ans = new alien_value_array(this->alientype());
    memcpy(ans->ptr(), this->ptr(), this->alientype()->__sizeof());
    return ans;
}

/* static */
alien_value* alien_value_array::from_lua(const alien_type* type, lua_State* L, int idx)
{
    if (!alien_isarray(L, idx)) {
        luaL_typerror(L, idx, "alien: require array");
        return nullptr;
    }

    alien_value_array* obj = alien_checkarray(L, idx);
    return new alien_value_array(obj->alientype(), obj->_mem, obj->ptr());
}

/* static */
alien_value* alien_value_array::from_ptr(const alien_type* type, lua_State* L, void* ptr)
{
    auto ans = new alien_value_array(type);
    memcpy(ans->ptr(), ptr, ans->__sizeof());
    return ans;
}

/** static */
alien_value* alien_value_array::from_shr(const alien_type* type, lua_State* L, std::shared_ptr<char> mem, void* ptr) {
    return new alien_value_array(type, mem, ptr);
}

/* static */
alien_value* alien_value_array::new_value(const alien_type* type, lua_State* L)
{
    return new alien_value_array(type);
}

/* static */
bool alien_value_array::is_this_value(const alien_type* type, lua_State* L, int idx)
{
    return alien_isarray(L, idx);
}

/* static */
alien_value_array* alien_value_array::checkvalue(const alien_type* type, lua_State* L, int idx)
{
    return alien_checkarray(L, idx);
}

