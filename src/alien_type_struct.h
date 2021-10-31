#ifndef _ALIEN_TYPE_STRUCT_H_
#define _ALIEN_TYPE_STRUCT_H_

#include "alien.h"
#include "alien_type.h"
#include <string>
#include <vector>
#include <memory>
#include <map>


class alien_type_struct: public alien_type {
    private:
        std::map<std::string,std::pair<size_t,alien_type*>> members;
        ffi_abi abi;
        ffi_type* pffi_type;

        struct _member_info {
            const alien_type* type;
            size_t offset;

            _member_info(const alien_type* type, size_t offset)
                : type(type), offset(offset)
            {}
        };

    public:
        alien_type_struct(const std::string& type_name, 
                          ffi_abi abi, 
                          const std::vector<std::pair<std::string, alien_type*>>& members);

        virtual ffi_type* ffitype() override;
        virtual alien_value* from_lua(lua_State* L, int idx) const override;
        virtual alien_value* from_ptr(lua_State* L, void*) const override;
        virtual alien_value* from_shr(lua_State* L, 
                                      std::shared_ptr<char> m,
                                      void* ptr) const override;
        virtual alien_value* new_value(lua_State* L) const override;

        std::unique_ptr<_member_info> member_info(const std::string& member) const;

        virtual bool is_this_type(lua_State* L, int idx) const override;
        virtual alien_value* checkvalue(lua_State* L, int idx) const override;

        virtual bool is_struct() const override;
        virtual ~alien_type_struct() override;
};

#endif // _ALIEN_TYPE_STRUCT_H_
