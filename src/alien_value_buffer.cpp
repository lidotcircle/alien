#include "alien_value_buffer.h"
#include <string.h>
#include <assert.h>
using namespace std;

#define ALIEN_VALUE_BUFFER_META "alien_value_buffer_meta"


alien_value_buffer::alien_value_buffer(class alien_type* type, size_t n):
    alien_value(type, nullptr), pptr(nullptr), buf_len(n)
{
    if (n > 0) {
        this->_mem = shared_ptr<char>(new char[n], [](char* ptr) {delete ptr;});
        pptr = this->_mem.get();
    }
}

static bool in_range(size_t len, size_t s, size_t idx, size_t off) {
    return (s * (idx + 1) + off <= len);
}

#define MENTRY(n, t) t alien_value_buffer::get_##n(lua_State* L, size_t nn, size_t off) const { \
    if (!in_range(this->buf_len, sizeof(t), nn, off)) \
        return luaL_error(L, "alien: access buffer out of range (type = %s)", #t); \
    void* ptr = static_cast<char*>(this->pptr) + off;\
    return static_cast<t*>(ptr)[nn]; \
}
buffer_access_type
#undef MENTRY

#define MENTRY(n, t) void alien_value_buffer::set_##n(lua_State* L, size_t nn, size_t off, t val) { \
    if (!in_range(this->buf_len, sizeof(t), nn, off)) {\
        luaL_error(L, "alien: access buffer out of range (type = %s)", #t); \
        return; \
    } \
    void* ptr = static_cast<char*>(this->pptr) + off;\
    static_cast<t*>(ptr)[nn] = val; \
}
buffer_access_type
#undef MENTRY

void* alien_value_buffer::ptr() {
    return &this->pptr;
}

const void* alien_value_buffer::ptr() const {
    return &this->pptr;
}

void alien_value_buffer::assignFrom(const alien_value& val) {
    if (val.alientype() != this->alientype())
        throw std::runtime_error("alien: assignment between incompatible type");

    const alien_value_buffer* valb = dynamic_cast<const alien_value_buffer*>(&val);
    if (valb == nullptr)
        throw std::runtime_error("alien: nope nope buffer error");

    this->buf_len = valb->buf_len;
    this->_mem = std::shared_ptr<char>(new char[valb->buf_len], [](char* ptr) {delete[] ptr;});
    this->pptr = this->_mem.get();
    memcpy(this->_mem.get(), valb->_mem.get(), this->buf_len);
}

void alien_value_buffer::to_lua(lua_State* L) const {
    auto vv = new alien_value_buffer(*this);
    alien_value_buffer** pvv = (alien_value_buffer**)lua_newuserdata(L, sizeof(alien_value_buffer*)); 
    *pvv = vv;
    luaL_setmetatable(L, ALIEN_VALUE_BUFFER_META);
}

alien_value* alien_value_buffer::copy() const {
    auto vv = new alien_value_buffer(*this);
    vv->_mem = std::shared_ptr<char>(new char[vv->buf_len], [](char* ptr) {delete[] ptr;});
    vv->pptr = vv->_mem.get();
    memcpy(vv->_mem.get(), this->_mem.get(), this->buf_len);
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
bool alien_value_buffer::is_this_value(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_buffer());
    return alien_isbuffer(L, idx);
}

/** static */
alien_value_buffer* alien_value_buffer::checkvalue(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_buffer());
    return alien_checkbuffer(L, idx);
}

