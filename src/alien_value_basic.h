#ifndef _ALIEN_VALUE_BASIC_H_
#define _ALIEN_VALUE_BASIC_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_basic: public alien_value {
    public:
        alien_value_basic(const alien_type* type);
        alien_value_basic(const alien_type* type, void* ptr);
        alien_value_basic(const alien_type* type, std::shared_ptr<char> mem, void* ptr);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_basic* checkvalue(const alien_type* type, lua_State* L, int idx);
};


int alien_value_basic_init(lua_State* L);

#endif // _ALIEN_VALUE_BASIC_H_
