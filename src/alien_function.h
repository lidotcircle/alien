#ifndef _ALIEN_FUNCTION_H_
#define _ALIEN_FUNCTION_H_

#include <lua.hpp>
#include <ffi.h>
#include <string>
#include <vector>
#include "alien_type.h"
#include "alien_library.h"


class alien_Function {
    private:
        alien_Library *lib;
        std::string name;

        void *fn;
        alien_type* ret_type;
        std::vector<alien_type*> params;
        ffi_cif cif;

        /* hook part */
        void* hookhandle;
        int   trampoline_ref;
        int   keepalive_ref;

    public:
        alien_Function() = delete;
        alien_Function(const alien_Function&) = delete;
        alien_Function(alien_Function&&) = delete;
        alien_Function& operator=(const alien_Function&) = delete;
        alien_Function& operator=(alien_Function&&) = delete;

        alien_Function(alien_Library* lib, void* fn, const std::string& name);

        bool define_types(ffi_abi abi, alien_type* ret, const std::vector<alien_type*>& params);
        int  call_from_lua(lua_State *L);

        void hook(lua_State* L, void* jmpto, int objref);
        void unhook(lua_State* L);
        int  trampoline(lua_State* L);

        std::string tostring();
        void* funcaddr();

        ~alien_Function();
};


int alien_function__make_function(lua_State *L, alien_Library* lib, void *fn, const std::string& name);

int alien_function_new(lua_State *L);

extern const ffi_abi ffi_abis[];
extern const char *const ffi_abi_names[];

#endif // _ALIEN_FUNCTION_H_
