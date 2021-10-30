#ifndef _ALIEN_VALUE_UNION_H_
#define _ALIEN_VALUE_UNION_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_union: public alien_value {
    public:
        alien_value_union(const alien_type* type);
        alien_value_union(const alien_type* type, void* ptr);
        alien_value_union(const alien_type* type, std::shared_ptr<char> _mem, void* ptr);

        alien_value* get_member(const std::string& member);
        const alien_value* get_member(const std::string& member) const;

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_union* checkvalue(const alien_type* type, lua_State* L, int idx);
};

int alien_value_union_init(lua_State* L);

#endif // _ALIEN_VALUE_UNION_H_
