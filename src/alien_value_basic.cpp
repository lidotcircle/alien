#include "alien_value_basic.h"
#include <string.h>


alien_value_basic::alien_value_basic(const alien_type* type): alien_value(type) {}
alien_value_basic::alien_value_basic(const alien_type* type, void* ptr): alien_value(type, ptr) {}

void alien_value_basic::toLua(lua_State* L) const {
    if (this->type->is_integer()) {
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

alien_value* alien_value_basic::copy() const {
    auto ans = new alien_value_basic(this->type);
    memcpy(ans->ptr(), const_cast<void*>(this->ptr()), this->__sizeof());

    return ans;
}

/** static */
alien_value* alien_value_basic::fromLua(const alien_type* type, lua_State* L, int idx) {
    if (!lua_isnumber(L, idx)) {
        luaL_error(L, "alien: expect number");
        return nullptr;
    }

    long long ival = lua_tointeger(L, idx);
    double fval = lua_tonumber(L, idx);
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
                luaL_error(L, "alien: unexpected integer size");
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
                luaL_error(L, "alien: unexpected integer size");
        }
    } else if (type->is_float()) {
        *static_cast<float*>(ans->ptr()) = fval;
    } else if (type->is_double()) {
        *static_cast<double*>(ans->ptr()) = fval;
    } else if (type->is_rawpointer()) {
        *static_cast<void**>(ans->ptr()) = reinterpret_cast<void*>(ival);
    } else {
        luaL_error(L, "alien: unexpected basic type");
    }

    return ans;
}

