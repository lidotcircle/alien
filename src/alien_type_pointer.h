#ifndef _ALIEN_TYPE_POINTER_H_
#define _ALIEN_TYPE_POINTER_H_

#include "alien.h"
#include "alien_type.h"

class alien_type_pointer: public alien_type {
    private:
        alien_type* _ptr_type;

    public:
        alien_type_pointer(alien_type* _ptr_type);

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* new_value(lua_State* L) const override;
        const alien_type* ptr_type() const;
        alien_type* ptr_type();

        virtual bool is_pointer() const override;
};

#endif // _ALIEN_TYPE_POINTER_H_
