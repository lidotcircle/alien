#include "alien_value_union.h"
#include "alien_type_union.h"
#include <string>
#include <string.h>
using namespace std;


#define ALIEN_VALUE_UNION_META "alien_value_union_meta"


static bool alien_isunion(lua_State* L, int idx)
{
    return luaL_testudata(L, idx, ALIEN_VALUE_UNION_META) != nullptr;
}

static alien_value_union* alien_checkunion(lua_State* L, int idx)
{
    return *static_cast<alien_value_union**>(luaL_checkudata(L, idx, ALIEN_VALUE_UNION_META));
}

static int alien_value_union_gc(lua_State* L);
static int alien_value_union_tostring(lua_State* L);
static int alien_value_union_index(lua_State* L);
static int alien_value_union_setindex(lua_State* L);

int alien_value_union_init(lua_State* L)
{
    luaL_newmetatable(L, ALIEN_VALUE_UNION_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_union_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_union_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_value_union_index);
    lua_settable(L, -3);

    lua_pushliteral(L, "__setindex");
    lua_pushcfunction(L, alien_value_union_setindex);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

static int alien_value_union_gc(lua_State* L)
{
    alien_value_union* s = alien_checkunion(L, 1);
    delete s;
    return 0;
}
static int alien_value_union_tostring(lua_State* L)
{
    alien_value_union* s = alien_checkunion(L, 1);
    lua_pushfstring(L, "[alien union: %p]", s);
    return 1;
}
static int alien_value_union_index(lua_State* L)
{
    alien_value_union* s = alien_checkunion(L, 1);
    const char* key = luaL_checkstring(L, 2);

   std::unique_ptr<alien_value> mem(s->get_member(key));
    if (mem == nullptr)
        return luaL_error(L, "union as no member %s", key);

    mem->to_lua(L);
    return 1;
}
static int alien_value_union_setindex(lua_State* L)
{
    alien_value_union* s = alien_checkunion(L, 1);
    const char* key = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> mb(s->get_member(key));

    if (mb == nullptr)
        return luaL_error(L, "union as no member %s", key);

    mb->assignFromLua(L, 3);
    return 0;
}

alien_value_union::alien_value_union(const alien_type* type): alien_value(type)
{
}

alien_value_union::alien_value_union(const alien_type* type, void* ptr): alien_value(type, ptr)
{
}

alien_value_union::alien_value_union(const alien_type* type, std::shared_ptr<char> mem, void* ptr):
    alien_value(type, mem, ptr)
{
}

alien_value* alien_value_union::get_member(const string& m) 
{
    const alien_type_union* t = dynamic_cast<const alien_type_union*>(this->alientype());
    if (t == nullptr)
        return nullptr;

    auto mt = t->member_type(m);
    if (mt == nullptr)
        return nullptr;

    // TODO
    return mt->from_ptr(nullptr, this->_mem, this->ptr());
}

const alien_value* alien_value_union::get_member(const string& m) const
{
    return const_cast<alien_value_union*>(this)->get_member(m);
}

void alien_value_union::to_lua(lua_State* L) const
{
    alien_value_union* obj = 
        new alien_value_union(this->alientype(), this->_mem, const_cast<void*>(this->ptr()));
    alien_value_union** p = static_cast<alien_value_union**>(lua_newuserdata(L, sizeof(alien_value_union*)));
    *p = obj;
    luaL_getmetatable(L, ALIEN_VALUE_UNION_META);
}

alien_value* alien_value_union::copy() const {
    alien_value_union* ans = new alien_value_union(this->alientype());
    memcpy(ans->ptr(), this->ptr(), this->alientype()->__sizeof());
    return ans;
}

/* static */
alien_value* alien_value_union::from_lua(const alien_type* type, lua_State* L, int idx)
{
    if (!alien_isunion(L, idx)) {
        luaL_typerror(L, idx, ALIEN_VALUE_UNION_META);
        return nullptr;
    }

    alien_value_union* obj = alien_checkunion(L, idx);
    return new alien_value_union(obj->alientype(), obj->_mem, obj->ptr());
}

/* static */
alien_value* alien_value_union::from_ptr(const alien_type* type, lua_State* L, void* ptr)
{
    return new alien_value_union(type, ptr);
}

/* static */
alien_value* alien_value_union::new_value(const alien_type* type, lua_State* L)
{
    return new alien_value_union(type);
}

/* static */
bool alien_value_union::is_this_value(const alien_type* type, lua_State* L, int idx)
{
    return alien_isunion(L, idx);
}

/* static */
alien_value_union* alien_value_union::checkvalue(const alien_type* type, lua_State* L, int idx)
{
    return alien_checkunion(L, idx);
}

