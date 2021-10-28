#ifndef _ALIEN_TYPE_CALLBACK_H_
#define _ALIEN_TYPE_CALLBACK_H_

#include "alien.h"
#include "alien_type.h"

class alien_type_callback: public alien_type {
    public:
        alien_type_callback();

        virtual ffi_type* ffitype() override;
        virtual alien_value* fromLua(lua_State* L, int idx) const override;
        virtual alien_value* new_value() const override;

        virtual bool is_callback() const override;
};

#endif // _ALIEN_TYPE_CALLBACK_H_
