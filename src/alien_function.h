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
    private:
        alien_Library *lib;
        std::string name;
        lua_State* L;
        bool is_variadic;

        void *fn;
        alien_type* ret_type;
        std::vector<alien_type*> params;
        ffi_cif cif;
        std::unique_ptr<ffi_type*[]> ffi_params;

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

        bool define_types(ffi_abi abi, alien_type* ret, const std::vector<alien_type*>& params, bool is_variadic);
        int  call_from_lua(lua_State *L);

        int hook(lua_State* L, void* jmpto, int objref);
        int unhook(lua_State* L);
        int trampoline(lua_State* L);

        std::string tostring();
        void* funcaddr();

        ~alien_Function();
};


int alien_function_init(lua_State *L);

int alien_function__make_function(lua_State *L, alien_Library* lib, void *fn, const std::string& name);
ffi_abi alien_checkabi(lua_State* L, int idx);
std::tuple<ffi_abi,alien_type*,std::vector<alien_type*>,bool> alien_function__parse_types_table(lua_State *L, int idx);

ALIEN_LUA_FUNC int alien_function_new(lua_State *L);

#endif // _ALIEN_FUNCTION_H_
