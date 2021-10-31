#ifndef _alien_type_basic_BASIC_H_
#define _alien_type_basic_BASIC_H_

#include <ffi.h>
#include <string>
#include "alien.h"
#include "alien_type.h"

class alien_type_basic: public alien_type {
    private:
        ffi_abi abi;
        ffi_type* pffi_type;

    public:
        alien_type_basic(const std::string& type_name, ffi_abi abi, ffi_type*);

        virtual ffi_type* ffitype() override;

        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void* ptr) const override;
        virtual alien_value* from_shr(lua_State* L, 
                                      std::shared_ptr<char> m,
                                      void* ptr) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        virtual bool is_integer()    const override;
        virtual bool is_signed()     const override;
        virtual bool is_float()      const override;
        virtual bool is_double()     const override;
        virtual bool is_rawpointer() const override;
        virtual bool is_void()       const override;
        virtual bool is_basic()      const override;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;

        virtual ~alien_type_basic() override;
};

#endif // _alien_type_basic_BASIC_H_
