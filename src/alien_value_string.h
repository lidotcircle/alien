#ifndef _ALIEN_VALUE_STRING_H_
#define _ALIEN_VALUE_STRING_H_

#include "alien.h"
#include "alien_value.h"
#include <memory>


class alien_value_string: public alien_value {
    private:
        std::shared_ptr<char> str;
        size_t _strlen;

    public:
        alien_value_string(const alien_type* type);
        alien_value_string(const alien_type* type, const char* ptr);
        alien_value_string(const alien_value_string& other);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_string* checkvalue(const alien_type* type, lua_State* L, int idx);
};


int alien_value_string_init(lua_State* L);
int alien_value_string_new(lua_State* L);

#endif // _ALIEN_VALUE_STRING_H_
