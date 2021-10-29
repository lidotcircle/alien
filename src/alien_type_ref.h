#ifndef _ALIEN_TYPE_REF_H_
#define _ALIEN_TYPE_REF_H_

#include "alien.h"
#include "alien_type.h"

class alien_type_ref: public alien_type {
    public:
        alien_type_ref();

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        virtual bool is_ref() const override;
};

#endif // _ALIEN_TYPE_REF_H_
