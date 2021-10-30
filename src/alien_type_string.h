#ifndef _ALIEN_TYPE_STRING_H_
#define _ALIEN_TYPE_STRING_H_

#include "alien.h"
#include "alien_type.h"

class alien_type_string: public alien_type {
    public:
        alien_type_string();

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;

        virtual bool is_string() const override;
};

#endif // _ALIEN_TYPE_STRING_H_
