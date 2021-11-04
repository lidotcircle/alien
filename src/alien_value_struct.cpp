#include "alien_value_struct.h"
#include "alien_type_struct.h"
#include "alien_exception.h"
#include <string>
#include <string.h>
using namespace std;


#define ALIEN_VALUE_STRUCT_META "alien_value_struct_meta"


static bool alien_isstruct(lua_State* L, int idx)
{
    return luaL_testudata(L, idx, ALIEN_VALUE_STRUCT_META) != nullptr;
}

static alien_value_struct* alien_checkstruct(lua_State* L, int idx)
{
    return *static_cast<alien_value_struct**>(luaL_checkudata(L, idx, ALIEN_VALUE_STRUCT_META));
}

static int alien_value_struct_gc(lua_State* L);
static int alien_value_struct_tostring(lua_State* L);
static int alien_value_struct_index(lua_State* L);
static int alien_value_struct_newindex(lua_State* L);

int alien_value_struct_init(lua_State* L)
{
    luaL_newmetatable(L, ALIEN_VALUE_STRUCT_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_struct_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_struct_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_value_struct_index);
    lua_settable(L, -3);

    lua_pushliteral(L, "__newindex");
    lua_pushcfunction(L, alien_value_struct_newindex);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

int alien_value_struct_new(lua_State* L) {
    alien_type* structtype = alien_checktype(L, 1);
    std::unique_ptr<alien_value> val(structtype->new_value(L));
    val->to_lua(L);

    return 1;
}

static int alien_value_struct_gc(lua_State* L)
{
    alien_value_struct* s = alien_checkstruct(L, 1);
    delete s;
    return 0;
}
static int alien_value_struct_tostring(lua_State* L)
{
    alien_value_struct* s = alien_checkstruct(L, 1);
    lua_pushfstring(L, "[alien struct: %p]", s);
    return 1;
}
static int alien_value_struct_index(lua_State* L)
{
    alien_value_struct* s = alien_checkstruct(L, 1);
    const char* key = luaL_checkstring(L, 2);

    std::unique_ptr<alien_value> mem(s->get_member(key));
    if (mem == nullptr)
        throw AlienException("struct has no member %s", key);

    mem->to_lua(L);
    return 1;
}
static int alien_value_struct_newindex(lua_State* L)
{
    alien_value_struct* s = alien_checkstruct(L, 1);
    const char* key = luaL_checkstring(L, 2);
    std::unique_ptr<alien_value> mb(s->get_member(key));

    if (mb == nullptr)
        throw AlienException("struct has no member %s", key);

    mb->assignFromLua(L, 3);
    return 0;
}

alien_value_struct::alien_value_struct(const alien_type* type): alien_value(type)
{
}

alien_value_struct::alien_value_struct(const alien_type* type, std::shared_ptr<char> mem, void* ptr):
    alien_value(type, mem, ptr)
{
}

alien_value* alien_value_struct::get_member(const string& m) 
{
    const alien_type_struct* t = dynamic_cast<const alien_type_struct*>(this->alientype());
    if (t == nullptr)
        return nullptr;

    auto minfo = t->member_info(m);
    if (minfo == nullptr)
        return nullptr;

    auto mt = minfo->type;
    void* ptr = static_cast<char*>(this->ptr()) + minfo->offset;

    // TODO
    return mt->from_shr(nullptr, this->_mem, ptr);
}

const alien_value* alien_value_struct::get_member(const string& m) const
{
    return const_cast<alien_value_struct*>(this)->get_member(m);
}

void alien_value_struct::to_lua(lua_State* L) const
{
    alien_value_struct* obj = 
        new alien_value_struct(this->alientype(), this->_mem, const_cast<void*>(this->ptr()));
    alien_value_struct** p = static_cast<alien_value_struct**>(lua_newuserdata(L, sizeof(alien_value_struct*)));
    *p = obj;
    luaL_setmetatable(L, ALIEN_VALUE_STRUCT_META);
}

alien_value* alien_value_struct::copy() const {
    alien_value_struct* ans = new alien_value_struct(this->alientype());
    memcpy(ans->ptr(), this->ptr(), this->alientype()->__sizeof());
    return ans;
}

/* static */
alien_value* alien_value_struct::from_lua(const alien_type* type, lua_State* L, int idx)
{
    if (!alien_isstruct(L, idx)) {
        luaL_typerror(L, idx, ALIEN_VALUE_STRUCT_META);
        return nullptr;
    }

    alien_value_struct* obj = alien_checkstruct(L, idx);
    return new alien_value_struct(obj->alientype(), obj->_mem, obj->ptr());
}

/* static */
alien_value* alien_value_struct::from_ptr(const alien_type* type, lua_State* L, void* ptr)
{
    auto ans = new alien_value_struct(type);
    memcpy(ans->ptr(), ptr, ans->__sizeof());
    return ans;
}

/** static */
alien_value* alien_value_struct::from_shr(const alien_type* type, lua_State* L, std::shared_ptr<char> mem, void* ptr) {
    return new alien_value_struct(type, mem, ptr);
}

/* static */
alien_value* alien_value_struct::new_value(const alien_type* type, lua_State* L)
{
    return new alien_value_struct(type);
}

/* static */
bool alien_value_struct::is_this_value(const alien_type* type, lua_State* L, int idx)
{
    return alien_isstruct(L, idx);
}

/* static */
alien_value_struct* alien_value_struct::checkvalue(const alien_type* type, lua_State* L, int idx)
{
    return alien_checkstruct(L, idx);
}

