#ifndef _ALIEN_VALUE_H_
#define _ALIEN_VALUE_H_

#include <lua.hpp>
#include <memory>
#include <string>
#include "alien_type_basic.h"

/* push a alien vlaue into stack */
int alien_value_from_type(lua_State* L, alien_type* type);


class alien_value {
    private:
        const alien_type* type;
        std::shared_ptr<char[]> _mem;
        void* val_ptr;

    public:
        alien_value() = delete;
        alien_value(const alien_type* type);

        void* ptr();
        size_t __sizeof();

        void assignto(const std::string& member, const alien_value& val);
        alien_value getmember(const std::string& member);

        int assignLuaValue(lua_State* L, const std::string& member);
        int getMember(lua_State*L, const std::string& member);

        alien_value copy();
};

#endif // _ALIEN_VALUE_H_
