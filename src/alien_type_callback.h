#ifndef _ALIEN_TYPE_CALLBACK_H_
#define _ALIEN_TYPE_CALLBACK_H_

#include "alien.h"
#include "alien_type.h"

class alien_type_callback: public alien_type {
    public:
        alien_type_callback();

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* from_shr(lua_State* L, 
                                      std::shared_ptr<char> m,
                                      void* ptr) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        virtual bool is_callback() const override;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;
};

#endif // _ALIEN_TYPE_CALLBACK_H_
