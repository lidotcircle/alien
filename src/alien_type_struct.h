#ifndef _ALIEN_TYPE_STRUCT_H_
#define _ALIEN_TYPE_STRUCT_H_

#include "alien.h"
#include "alien_type.h"
#include <string>
#include <vector>


class alien_type_struct: public alien_type {
    private:
        std::vector<std::pair<std::string, alien_type*>> members;
        ffi_abi abi;
        ffi_type* pffi_type;
        size_t* member_offs;

    public:
        alien_type_struct(const std::string& type_name, 
                          ffi_abi abi, 
                          const std::vector<std::pair<std::string, alien_type*>>& members);

        virtual ffi_type* ffitype() override;
        virtual alien_value* fromLua(lua_State* L, int idx) const override;
        virtual alien_value* new_value() const override;

        bool   has_member(const std::string& member);
        size_t __offsetof(const std::string& member);

        virtual bool is_struct() const override;
        virtual ~alien_type_struct() override;
};

#endif // _ALIEN_TYPE_STRUCT_H_
