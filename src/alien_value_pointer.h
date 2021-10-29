#ifndef _ALIEN_VALUE_POINTER_H_
#define _ALIEN_VALUE_POINTER_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_pointer: public alien_value {
    public:
        alien_value_pointer(const alien_type* type, const alien_type* ref_type);
        alien_value_pointer(const alien_type* type, const alien_type* ref_type, void* ptr);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);
};

#endif // _ALIEN_VALUE_POINTER_H_
