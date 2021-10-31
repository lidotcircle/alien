#include "alien_value_string.h"
#include <string.h>
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

    memcpy(this->ptr(), ptr, this->_strlen + 1);
}

alien_value_string::alien_value_string(const alien_value_string& other):
    alien_value(other.alientype()), str(other.str), _strlen(other._strlen)
{
    *static_cast<void**>(this->ptr()) = this->str.get();
}

void alien_value_string::to_lua(lua_State* L) const {
    lua_pushstring(L, static_cast<const char*>(this->ptr()));
}

alien_value* alien_value_string::copy() const {
    return new alien_value_string(*this);
}

/** static */
alien_value* alien_value_string::from_lua(const alien_type* type, lua_State* L, int idx) {
    const char* str = lua_tostring(L, idx);
    return new alien_value_string(type, str);
}
alien_value* alien_value_string::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
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
    std::unique_ptr<alien_value> val(stringtype->from_lua(L, 1));
    val->to_lua(L);

    return 1;
}

