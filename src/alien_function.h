#ifndef _ALIEN_FUNCTION_H_
#define _ALIEN_FUNCTION_H_

#include <ffi.h>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include "alien.h"
#include "alien_type.h"
#include "alien_library.h"


class alien_Function {
    public:
        alien_Function() = delete;
        alien_Function(const alien_Function&) = delete;
        alien_Function(alien_Function&&) = delete;
        alien_Function& operator=(const alien_Function&) = delete;
        alien_Function& operator=(alien_Function&&) = delete;

        alien_Function(alien_Library* lib, void* fn, const std::string& name);

        bool define_types(ffi_abi abi, alien_type* ret, const std::vector<alien_type*>& params, bool is_variadic);
        int  call_from_lua(lua_State *L);

        int hook(lua_State* L, void* jmpto, int objref);
        int unhook(lua_State* L);
        int trampoline(lua_State* L);

        std::string tostring();
        void* funcaddr();

        ~alien_Function();
};

#endif // _ALIEN_FUNCTION_H_
