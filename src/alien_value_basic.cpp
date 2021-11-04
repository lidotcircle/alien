#include "alien_type.h"
#include "alien_value_basic.h"
#include "alien_exception.h"
#include <memory>
#include <assert.h>
#include <string.h>
using namespace std;

#define ALIEN_VALUE_BASIC_META "alien_value_basic_meta"

static bool alien_isbasic(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_VALUE_BASIC_META) != nullptr;
}
static alien_value_basic* alien_checkbasic(lua_State* L, int idx) {
    alien_value** v = static_cast<alien_value**>(luaL_checkudata(L, idx, ALIEN_VALUE_BASIC_META));
    auto rv = dynamic_cast<alien_value_basic*>(*v);
    assert(rv != nullptr);
    return rv;
}

alien_value_basic::alien_value_basic(const alien_type* type): alien_value(type) {}
alien_value_basic::alien_value_basic(const alien_type* type, std::shared_ptr<char> mem, void* ptr): alien_value(type, mem, ptr) {}

void alien_value_basic::to_lua(lua_State* L) const {
    if (this->type->is_signed()) {
        switch (this->__sizeof()) {
            case 1:
                lua_pushnumber(L, *static_cast<const int8_t *>(this->ptr())); break;
            case 2:
                lua_pushnumber(L, *static_cast<const int16_t*>(this->ptr())); break;
            case 4:
                lua_pushnumber(L, *static_cast<const int32_t*>(this->ptr())); break;
            case 8:
                lua_pushnumber(L, *static_cast<const int64_t*>(this->ptr())); break;
            default:
                luaL_error(L, "alien: bad signed integer type");
        }
    } else if (this->type->is_integer()) {
        switch (this->__sizeof()) {
            case 1:
                lua_pushnumber(L, *static_cast<const uint8_t *>(this->ptr())); break;
            case 2:
                lua_pushnumber(L, *static_cast<const uint16_t*>(this->ptr())); break;
            case 4:
                lua_pushnumber(L, *static_cast<const uint32_t*>(this->ptr())); break;
            case 8:
                lua_pushnumber(L, *static_cast<const uint64_t*>(this->ptr())); break;
            default:
                luaL_error(L, "alien: bad integer type");
        }
    } else if (this->type->is_float()) {
        lua_pushnumber(L, *static_cast<const float*>(this->ptr()));
    } else if (this->type->is_double()) {
        lua_pushnumber(L, *static_cast<const double*>(this->ptr()));
    } else if (this->type->is_rawpointer()) {
        void* ptr = *static_cast<void* const*>(this->ptr());
        lua_pushnumber(L, reinterpret_cast<size_t>(ptr));
    } else if (this->type->is_void()) {
    } else {
        luaL_error(L, "alien: bad basic type");
    }
}

void alien_value_basic::just_box(lua_State* L) const {
    auto value = this->copy();

    alien_value** ud = static_cast<alien_value**>(lua_newuserdata(L, sizeof(alien_value*)));
    *ud = value;
    luaL_setmetatable(L, ALIEN_VALUE_BASIC_META);
}

alien_value* alien_value_basic::copy() const {
    auto ans = new alien_value_basic(this->type);
    memcpy(ans->ptr(), const_cast<void*>(this->ptr()), this->__sizeof());

    return ans;
}

/** static */
alien_value* alien_value_basic::from_lua(const alien_type* type, lua_State* L, int idx) {
    if (alien_isbasic(L, idx)) {
        auto ans = alien_checkbasic(L, idx)->copy();
        if (ans->alientype() != type)
            throw AlienException("basic type mismatch");
        return ans;
    }

    long long ival;
    double fval;
    if (lua_isnumber(L, idx)) {
        ival = lua_tointeger(L, idx);
        fval = lua_tonumber(L, idx);
    } else if (type->is_rawpointer() && lua_islightuserdata(L, idx)) {
        ival = reinterpret_cast<long long>(lua_touserdata(L, idx));
    } else {
        throw AlienException("bad basic type, require a lua number "
                             "or light userdata for rawpointer type");
    }

    auto ans = new alien_value_basic(type);
    if (type->is_signed()) {
        switch (type->__sizeof()) {
            case 1:
                *static_cast<int8_t* >(ans->ptr()) = ival; break;
            case 2:
                *static_cast<int16_t*>(ans->ptr()) = ival; break;
            case 4:
                *static_cast<int32_t*>(ans->ptr()) = ival; break;
            case 8:
                *static_cast<int64_t*>(ans->ptr()) = ival; break;
            default:
                throw AlienException("unexpected signed integer size");
        }
    } else if (type->is_integer()) {
        switch (type->__sizeof()) {
            case 1:
                *static_cast<uint8_t* >(ans->ptr()) = ival; break;
            case 2:
                *static_cast<uint16_t*>(ans->ptr()) = ival; break;
            case 4:
                *static_cast<uint32_t*>(ans->ptr()) = ival; break;
            case 8:
                *static_cast<uint64_t*>(ans->ptr()) = ival; break;
            default:
                throw AlienException("unexpected integer size");
        }
    } else if (type->is_float()) {
        *static_cast<float*>(ans->ptr()) = fval;
    } else if (type->is_double()) {
        *static_cast<double*>(ans->ptr()) = fval;
    } else if (type->is_rawpointer()) {
        *static_cast<void**>(ans->ptr()) = reinterpret_cast<void*>(ival);
    } else {
        throw AlienException("unexpected basic type");
    }

    return ans;
}

/** static */
alien_value* alien_value_basic::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    auto ans = new alien_value_basic(type);

    if (type->is_signed()) {
        switch (type->__sizeof()) {
            case 1:
                *static_cast<int8_t* >(ans->ptr()) = *static_cast<int8_t *>(ptr); break;
            case 2:
                *static_cast<int16_t*>(ans->ptr()) = *static_cast<int16_t*>(ptr); break;
            case 4:
                *static_cast<int32_t*>(ans->ptr()) = *static_cast<int32_t*>(ptr); break;
            case 8:
                *static_cast<int64_t*>(ans->ptr()) = *static_cast<int64_t*>(ptr); break;
            default:
                luaL_error(L, "alien: unexpected integer size");
        }
    } else if (type->is_integer()) {
        switch (type->__sizeof()) {
            case 1:
                *static_cast<uint8_t* >(ans->ptr()) = *static_cast<uint8_t *>(ptr); break;
            case 2:
                *static_cast<uint16_t*>(ans->ptr()) = *static_cast<uint16_t*>(ptr); break;
            case 4:
                *static_cast<uint32_t*>(ans->ptr()) = *static_cast<uint32_t*>(ptr); break;
            case 8:
                *static_cast<uint64_t*>(ans->ptr()) = *static_cast<uint64_t*>(ptr); break;
            default:
                luaL_error(L, "alien: unexpected integer size");
        }
    } else if (type->is_float()) {
        *static_cast<float*>(ans->ptr()) = *static_cast<float*>(ptr);
    } else if (type->is_double()) {
        *static_cast<double*>(ans->ptr()) = *static_cast<double*>(ptr);
    } else if (type->is_rawpointer()) {
        *static_cast<void**>(ans->ptr()) = *static_cast<void**>(ptr);
    } else {
        luaL_error(L, "alien: unexpected basic type");
    }

    return ans;
}

/** static */
alien_value* alien_value_basic::from_shr(const alien_type* type, lua_State* L, std::shared_ptr<char> mem, void* ptr) {
    return new alien_value_basic(type, mem, ptr);
}

alien_value* alien_value_basic::new_value(const alien_type* type, lua_State* L) {
    return new alien_value_basic(type);
}

/** static */
bool alien_value_basic::is_this_value(const alien_type* type, lua_State* L, int idx) {
    if (alien_isbasic(L, idx)) {
        auto val = alien_checkbasic(L, idx);
        return !type || val->alientype() == type;
    }

    assert(type->is_basic());
    return lua_isnumber(L, idx);
}

/** static */
alien_value_basic* alien_value_basic::checkvalue(const alien_type* type, lua_State* L, int idx) {
    if (alien_isbasic(L, idx)) {
        auto val = alien_checkbasic(L, idx);
        if (type && val->alientype() != type)
            throw AlienException("basic type mismatch");
        return dynamic_cast<alien_value_basic*>(val->copy());
    }

    assert(type->is_basic());
    if (!lua_isnumber(L, idx)) {
        luaL_error(L, "alien: expect number");
        return nullptr;
    }

    auto ans = dynamic_cast<alien_value_basic*>(type->from_lua(L, idx));
    assert(ans != nullptr);
    return ans;
}

static int alien_value_basic_gc(lua_State* L);
static int alien_value_basic_tostring(lua_State* L);
int alien_value_basic_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_VALUE_BASIC_META);

    lua_pushcfunction(L, alien_value_basic_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, alien_value_basic_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pop(L, 1);
    return 0;
}

static int alien_value_basic_gc(lua_State* L) {
    auto v = alien_checkbasic(L, 1);
    delete v;
    return 0;
}

static int alien_value_basic_tostring(lua_State* L) {
    auto v = alien_checkbasic(L, 1);
    v->to_lua(L);
    lua_tostring(L, -1);
    return 1;
}

int alien_value_basic_new(lua_State* L) {
    auto type = alien_checktype(L, 1);

    if (lua_gettop(L) > 1) {
        std::unique_ptr<alien_value> val(type->from_lua(L, 2));
        val->to_lua(L);
    } else {
        std::unique_ptr<alien_value> val(type->new_value(L));
        val->to_lua(L);
    }

    return 1;
}

