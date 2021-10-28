#ifndef _ALIEN_TYPE_BUFFER_H_
#define _ALIEN_TYPE_BUFFER_H_

#include <ffi.h>
#include "alien_type.h"

extern ffi_type ffi_type_buffer;


class alien_type_buffer: public alien_type {
    public:
        alien_type_buffer();

        virtual ffi_type* ffitype() override;
        virtual size_t __sizeof() const override;

        virtual alien_value* fromLua(lua_State* L, int idx) const override;
        virtual alien_value* new_value() const override;

        virtual bool is_buffer() const override;

        ~alien_type_buffer();
};

#endif // _ALIEN_TYPE_BUFFER_H_
