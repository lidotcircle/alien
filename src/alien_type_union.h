#ifndef _ALIEN_TYPE_UNION_H_
#define _ALIEN_TYPE_UNION_H_

#include "alien.h"
#include "alien_type_basic.h"
#include <string>
#include <vector>


class alien_type_union: public alien_type {
    private:
        std::vector<std::pair<std::string, alien_type*>> members;
        ffi_abi abi;
        ffi_type* pffi_type;

    public:
        alien_type_union(const std::string& type_name,
                         ffi_abi abi, 
                         const std::vector<std::pair<std::string,alien_type*>>& members);

        virtual ffi_type* ffitype() override;
        virtual alien_value* fromLua(lua_State* L, int idx) const override;
        virtual alien_value* new_value() const override;

        size_t sizeof_member(const std::string& member);

        virtual bool is_union() const override;
        virtual ~alien_type_union() override;
};

#endif // _ALIEN_TYPE_UNION_H_
