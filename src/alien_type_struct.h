#ifndef _ALIEN_TYPE_STRUCT_H_
#define _ALIEN_TYPE_STRUCT_H_

#include "alien_type_basic.h"
#include <string>
#include <vector>


class alien_type_struct: public alien_type {
    private:
        std::vector<std::pair<std::string, alien_type*>> members;
        size_t* member_offs;

    public:
        alien_type_struct(const std::string& type_name, ffi_abi abi, const std::vector<std::pair<std::string, alien_type*>>& members);

        bool has_member(const std::string& member);
        size_t __offsetof(const std::string& member);

        virtual ~alien_type_struct() override;
};

#endif // _ALIEN_TYPE_STRUCT_H_
