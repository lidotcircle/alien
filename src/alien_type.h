#ifndef _ALIEN_TYPE_H_
#define _ALIEN_TYPE_H_

#include <ffi.h>


class alien_type {
    protected:
        ffi_abi abi;
        ffi_type* pffi_type;

    public:
        alien_type(ffi_abi abi, ffi_type*);

        alien_type() = delete;
        alien_type(const alien_type&) = delete;
        alien_type(alien_type&&) = delete;
        alien_type& operator=(alien_type&&) = delete;
        alien_type& operator=(const alien_type&) = delete;

        virtual size_t __sizeof();
        virtual ~alien_type();
};

#endif // _ALIEN_TYPE_H_
