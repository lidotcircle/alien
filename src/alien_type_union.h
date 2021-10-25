#ifndef _ALIEN_TYPE_UNION_H_
#define _ALIEN_TYPE_UNION_H_

#include "alien_type_basic.h"
#include <string>
#include <vector>


class alien_type_union: public alien_type {
    private:
        std::vector<std::pair<std::string, alien_type*>> members;

    public:
        alien_type_union(const std::string& type_name, ffi_abi abi, const std::vector<std::pair<std::string,alien_type*>>& members);

        size_t sizeof_member(const std::string& member);

        virtual ~alien_type_union() override;
};

#endif // _ALIEN_TYPE_UNION_H_
