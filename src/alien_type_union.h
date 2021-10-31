#ifndef _ALIEN_TYPE_UNION_H_
#define _ALIEN_TYPE_UNION_H_

#include "alien.h"
#include "alien_type_basic.h"
#include <string>
#include <vector>
#include <map>


class alien_type_union: public alien_type {
    private:
        std::map<std::string, alien_type*> member_map;
        ffi_abi abi;
        ffi_type* pffi_type;

    public:
        alien_type_union(const std::string& type_name,
                         ffi_abi abi, 
                         const std::vector<std::pair<std::string,alien_type*>>& members);

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* from_shr(lua_State* L, 
                                      std::shared_ptr<char> m,
                                      void* ptr) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        const alien_type* member_type(const std::string& member) const;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;

        virtual bool is_union() const override;
        virtual ~alien_type_union() override;
};

#endif // _ALIEN_TYPE_UNION_H_
