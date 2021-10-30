#ifndef _ALIEN_VALUE_STRUCT_H_
#define _ALIEN_VALUE_STRUCT_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_struct: public alien_value {
    public:
        alien_value_struct(const alien_type* type, const alien_type* ref_type);
        alien_value_struct(const alien_type* type, const alien_type* ref_type, void* ptr);

        alien_value* get_member(const std::string& member);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_struct* checkvalue(const alien_type* type, lua_State* L, int idx);
};

#endif // _ALIEN_VALUE_STRUCT_H_
