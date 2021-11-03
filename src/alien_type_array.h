#ifndef _ALIEN_TYPE_ARRAY_H_
#define _ALIEN_TYPE_ARRAY_H_

#include "alien.h"
#include "alien_type.h"
#include <string>
#include <memory>


class alien_type_array: public alien_type {
    private:
        const alien_type* elem_type;
        ffi_type* pffi_type;
        ffi_abi abi;
        size_t elem_aligned_size;
        size_t len;

    public:
        alien_type_array(const alien_type* elemtype, size_t len, ffi_abi abi);

        virtual ffi_type*    ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* from_shr(lua_State* L, 
                                      std::shared_ptr<char> m,
                                      void* ptr) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        size_t array_length() const;
        size_t element_size() const;
        size_t element_aligned_size() const;
        const alien_type* elemtype() const;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;

        virtual bool is_array() const override;
        virtual ~alien_type_array() override;
};

#endif // _ALIEN_TYPE_ARRAY_H_
