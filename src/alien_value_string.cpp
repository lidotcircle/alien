#include "alien_value_string.h"
#include "alien_value_buffer.h"
#include <string.h>
#include <assert.h>
#include <memory>


alien_value_string::alien_value_string(const alien_type* type):
    alien_value(type), str(std::make_shared<char>(0)), _strlen(0)
{
    *static_cast<void**>(this->ptr()) = this->str.get();
}

alien_value_string::alien_value_string(const alien_type* type, const char* ptr):
    alien_value(type), str(), _strlen(0) 
{
    this->_strlen = strlen(ptr);
    this->str = std::shared_ptr<char>(new char[this->_strlen + 1], std::default_delete<char[]>());
    *static_cast<void**>(this->ptr()) = this->str.get();

    memcpy(*static_cast<void**>(this->ptr()), ptr, this->_strlen + 1);
}

alien_value_string::alien_value_string(const alien_value_string& other):
    alien_value(other.alientype()), str(other.str), _strlen(other._strlen)
{
    *static_cast<void**>(this->ptr()) = this->str.get();
}

void alien_value_string::to_lua(lua_State* L) const {
    lua_pushstring(L, *static_cast<char*const*>(this->ptr()));
}

alien_value* alien_value_string::copy() const {
    return new alien_value_string(*this);
}

/** static */
alien_value* alien_value_string::from_lua(const alien_type* type, lua_State* L, int idx) {
    alien_type* buf_type = alien_type_byname(L, "buffer");

    if (lua_isstring(L, idx)) {
        const char* str = lua_tostring(L, idx);
        assert(str != nullptr);
        return new alien_value_string(type, str);
    } else if (buf_type->is_this_type(L, idx)) {
        alien_value_buffer* buf = alien_value_buffer::checkvalue(buf_type, L, idx);
        char* cptr = *static_cast<char**>(buf->ptr());
        size_t slen = strnlen(cptr, buf->buflen());
        std::shared_ptr<char> str(new char[slen + 1], std::default_delete<char[]>());
        memcpy(str.get(), cptr, slen);
        str.get()[slen] = '\0';
        return new alien_value_string(type, str.get());
    } else {
        luaL_error(L, "alien: invalid type for string value");
        return nullptr;
    }
}
alien_value* alien_value_string::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    return new alien_value_string(type, *static_cast<const char**>(ptr));
}
alien_value* alien_value_string::from_shr(const alien_type* type, lua_State* L, std::shared_ptr<char> mem, void* ptr) {
    return new alien_value_string(type, *static_cast<const char**>(ptr));
}
alien_value* alien_value_string::new_value(const alien_type* type, lua_State* L) {
    return new alien_value_string(type);
}

bool alien_value_string::is_this_value(const alien_type* type, lua_State* L, int idx) {
    return lua_isstring(L, idx);
}
alien_value_string* alien_value_string::checkvalue(const alien_type* type, lua_State* L, int idx) {
    if (!lua_isstring(L, idx)) {
        luaL_error(L, "Expected string");
        return nullptr;
    }

    return new alien_value_string(type, lua_tostring(L, idx));
}

int alien_value_string_init(lua_State* L) {
    return 0;
}

int alien_value_string_new(lua_State* L) {
    alien_type* stringtype = alien_checktype(L, 1);
    if (lua_gettop(L) > 1) {
        luaL_argcheck(L, lua_isstring(L, 2), 2, "alien: require string");

        std::unique_ptr<alien_value> val(stringtype->from_lua(L, 2));
        val->to_lua(L);
    } else {
        std::unique_ptr<alien_value> val(stringtype->new_value(L));
        val->to_lua(L);
    }

    return 1;
}

