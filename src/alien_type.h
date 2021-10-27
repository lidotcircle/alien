#ifndef _ALIEN_TYPE_H_
#define _ALIEN_TYPE_H_

#include <lua.hpp>
#include <ffi.h>
#include <string>

/** forward declaration */
class alien_value;

class alien_type {
    private:
        std::string __tname;

    public:
        alien_type(const std::string& tname);

        alien_type() = delete;
        alien_type(const alien_type&) = delete;
        alien_type(alien_type&&) = delete;
        alien_type& operator=(alien_type&&) = delete;
        alien_type& operator=(const alien_type&) = delete;

        virtual ffi_type* ffitype() = 0;
        virtual const ffi_type* ffitype() const;
        virtual const std::string& __typename() const;
        virtual size_t __sizeof() const;

        virtual alien_value* fromLua(lua_State* L, int idx) const = 0;

        virtual bool is_integer() const;
        virtual bool is_signed() const;
        virtual bool is_float() const;
        virtual bool is_double() const;
        virtual bool is_rawpointer() const;
        virtual bool is_void() const;
        virtual bool is_basic() const;
        virtual bool is_ref() const;
        virtual bool is_pointer() const;
        virtual bool is_struct() const;
        virtual bool is_buffer() const;
        virtual bool is_union() const;

        virtual ~alien_type() = default;
};

/** initialization routine, call from openlib_*() function */
int alien_types_init(lua_State* L);

/** lua c functions */
int alien_types_defstruct(lua_State* L);
int alien_types_defunion(lua_State* L);
int alien_types_defref(lua_State* L);
int alien_types_defpointer(lua_State* L);
int alien_types_alias(lua_State* L);

#endif // _ALIEN_TYPE_H_
