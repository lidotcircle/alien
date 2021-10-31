#include "alien_value_buffer.h"
#include <string.h>
#include <assert.h>
using namespace std;

#define ALIEN_VALUE_BUFFER_META "alien_value_buffer_meta"


alien_value_buffer::alien_value_buffer(const alien_type* type, size_t n):
    alien_value(type), buf_len(n)
{
    if (n > 0)
        this->buf = shared_ptr<char>(new char[n], std::default_delete<char[]>());
    *static_cast<void**>(this->ptr()) = this->buf.get();
}

alien_value_buffer::alien_value_buffer(const alien_value_buffer& other):
    alien_value(other.alientype()), buf_len(0) 
{
    this->buf_len = other.buf_len;
    this->buf = other.buf;
    *static_cast<void**>(this->ptr()) = this->buf.get();
}

static bool in_range(size_t len, size_t s, size_t idx, size_t off) {
    return (s * (idx + 1) + off <= len);
}

#define MENTRY(n, t) t alien_value_buffer::get_##n(lua_State* L, size_t nn, size_t off) const { \
    if (!in_range(this->buf_len, sizeof(t), nn, off)) \
        return luaL_error(L, "alien: access buffer out of range (type = %s)", #t); \
    const void* ptr = *static_cast<char* const*>(this->ptr()) + off;\
    return static_cast<const t*>(ptr)[nn]; \
}
buffer_access_type
#undef MENTRY

#define MENTRY(n, t) void alien_value_buffer::set_##n(lua_State* L, size_t nn, size_t off, t val) { \
    if (!in_range(this->buf_len, sizeof(t), nn, off)) {\
        luaL_error(L, "alien: access buffer out of range (type = %s)", #t); \
        return; \
    } \
    void* ptr = *static_cast<char**>(this->ptr()) + off;\
    static_cast<t*>(ptr)[nn] = val; \
}
buffer_access_type
#undef MENTRY

void alien_value_buffer::assignFrom(const alien_value& val) {
    if (val.alientype() != this->alientype())
        throw std::runtime_error("alien: assignment between incompatible type");

    const alien_value_buffer* valb = dynamic_cast<const alien_value_buffer*>(&val);
    if (valb == nullptr)
        throw std::runtime_error("alien: nope nope buffer error");

    this->buf_len = valb->buf_len;
    this->buf = std::shared_ptr<char>(new char[valb->buf_len], std::default_delete<char[]>());
    memcpy(this->ptr(), valb->ptr(), this->buf_len);
    *static_cast<void**>(this->ptr()) = this->buf.get();
}

void alien_value_buffer::to_lua(lua_State* L) const {
    auto vv = new alien_value_buffer(*this);
    alien_value_buffer** pvv = (alien_value_buffer**)lua_newuserdata(L, sizeof(alien_value_buffer*)); 
    *pvv = vv;
    luaL_setmetatable(L, ALIEN_VALUE_BUFFER_META);
}

alien_value* alien_value_buffer::copy() const {
    auto vv = new alien_value_buffer(*this);
    vv->buf = std::shared_ptr<char>(new char[vv->buf_len], std::default_delete<char[]>());
    memcpy(vv->_mem.get(), this->_mem.get(), this->buf_len);
    *static_cast<void**>(vv->ptr()) = vv->buf.get();
    return vv;
}

static bool alien_isbuffer(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_VALUE_BUFFER_META) != nullptr;
}
static alien_value_buffer* alien_checkbuffer(lua_State* L, int idx) {
    return *static_cast<alien_value_buffer**>(luaL_checkudata(L, idx, ALIEN_VALUE_BUFFER_META));
}


static int alien_buffer_gc(lua_State* L);
#define MENTRY(n, t) static int alien_buffer_get_##n(lua_State* L); \
                     static int alien_buffer_set_##n(lua_State* L);
buffer_access_type
#undef MENTRY

int alien_value_buffer_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_VALUE_BUFFER_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_buffer_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_newtable(L);

#define MENTRY(n, t) \
    lua_pushliteral(L, #n"_get"); \
    lua_pushcfunction(L, alien_buffer_get_##n); \
    lua_settable(L, -3); \
    lua_pushliteral(L, #n"_set"); \
    lua_pushcfunction(L, alien_buffer_set_##n); \
    lua_settable(L, -3);
buffer_access_type
#undef MENTRY
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

int alien_value_buffer_new(lua_State* L) {
    auto type = alien_checktype(L, 1);
    assert(type->is_buffer());

    if (lua_gettop(L) == 1) {
        std::unique_ptr<alien_value> val(type->new_value(L));
        val->to_lua(L);
        return 1;
    }

    if (lua_isinteger(L, 2)) {
        size_t len = lua_tointeger(L, 2);
        if (len < 0)
            return luaL_error(L, "alien: buffer length must be positive");
        std::unique_ptr<alien_value> val(new alien_value_buffer(type, len));
        val->to_lua(L);
        return 1;
    } else if (lua_isstring(L, 2)) {
        size_t len = lua_rawlen(L, 2);
        len++;
        std::unique_ptr<alien_value> val(new alien_value_buffer(type, len));
        memcpy(*static_cast<char**>(val->ptr()), lua_tostring(L, 2), len);
        val->to_lua(L);
        return 1;
    } else {
        return luaL_error(L, "alien: invalid buffer constructor");
    }
}

static int alien_buffer_gc(lua_State* L) {
    alien_value_buffer* vb = alien_checkbuffer(L, 1);
    delete vb;
    return 0;
}
#define MENTRY(n, t) \
    static int alien_buffer_get_##n(lua_State* L) { \
        alien_value_buffer* vb = alien_checkbuffer(L, 1); \
        size_t index = luaL_checkinteger(L, 2); \
        size_t off = 0; \
        if (lua_gettop(L) > 2) \
            off = luaL_checkinteger(L, 3); \
        t val = vb->get_##n(L, index, off); \
        lua_pushnumber(L, val); \
        return 1; \
    } \
    static int alien_buffer_set_##n(lua_State* L) { \
        alien_value_buffer* vb = alien_checkbuffer(L, 1); \
        size_t index = luaL_checkinteger(L, 2); \
        t val = luaL_checknumber(L, 3); \
        size_t off = 0; \
        if (lua_gettop(L) > 3) \
            off = luaL_checkinteger(L, 4); \
        vb->set_##n(L, index, off, val); \
        return 0; \
    }
buffer_access_type
#undef MENTRY


/** static */
alien_value* alien_value_buffer::from_lua(const alien_type* type, lua_State* L, int idx) {
    if (!alien_isbuffer(L, idx))
        return nullptr;

    alien_value_buffer* vb = alien_checkbuffer(L, idx);
    return vb->copy();
}

/** static */
alien_value* alien_value_buffer::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    return nullptr;
}

/** static */
alien_value* alien_value_buffer::new_value(const alien_type* type, lua_State* L) {
    return nullptr;
}

/** static */
bool alien_value_buffer::is_this_value(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_buffer());
    return alien_isbuffer(L, idx);
}

/** static */
alien_value_buffer* alien_value_buffer::checkvalue(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_buffer());
    return alien_checkbuffer(L, idx);
}

