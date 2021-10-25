#ifndef _ALIEN_TYPE_BASIC_H_
#define _ALIEN_TYPE_BASIC_H_

#include <ffi.h>
#include <string>


class alien_type {
    protected:
        ffi_abi abi;
        ffi_type* pffi_type;
        std::string type_name;

    public:
        alien_type(const std::string& type_name, ffi_abi abi, ffi_type*);

        alien_type() = delete;
        alien_type(const alien_type&) = delete;
        alien_type(alien_type&&) = delete;
        alien_type& operator=(alien_type&&) = delete;
        alien_type& operator=(const alien_type&) = delete;

        ffi_type* ffitype();
        const std::string& __typename() const;

        virtual size_t __sizeof() const;
        virtual ~alien_type();
};

#endif // _ALIEN_TYPE_BASIC_H_
