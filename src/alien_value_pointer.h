#ifndef _ALIEN_VALUE_POINTER_H_
#define _ALIEN_VALUE_POINTER_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_pointer: public alien_value {
    private:
        alien_value* aval;
        alien_type* ptr_type;
        void init_value();

    public:
        alien_value_pointer(const alien_type* type);
        alien_value_pointer(const alien_type* type, void* ptr);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        alien_value* access_member(lua_State* L, const std::string&) const;
        alien_value* deref() const;
        bool is_null() const;

        virtual ~alien_value_pointer() override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_pointer* checkvalue(const alien_type* type, lua_State* L, int idx);
};


int alien_value_pointer_init(lua_State* L);
int alien_value_pointer_new(lua_State* L);

#endif // _ALIEN_VALUE_POINTER_H_
