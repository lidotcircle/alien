#ifndef _ALIEN_LIBRARY_H_
#define _ALIEN_LIBRARY_H_

#include <string>
#include <set>
#include "alien.h"

class alien_Function;


class alien_Library {
    private:
        void* lib;
        std::string name;
        lua_State* L;
        std::set<std::string> funclist;
        bool init_func_list;

    public:
        alien_Library() = delete;
        alien_Library(const alien_Library&) = delete;
        alien_Library(alien_Library&&) = delete;
        alien_Library& operator=(const alien_Library&) = delete;
        alien_Library& operator=(alien_Library&&) = delete;

        alien_Library(lua_State* L, const std::string& libname, void* libhandle);

        const std::set<std::string>& function_list();
        bool has_function(const std::string& func);
        lua_State* get_lua_State();

        void* loadfunc(const std::string& func);

        std::string tostring();
        const std::string& libname();

        ~alien_Library();
};

int alien_library_init(lua_State* L);

ALIEN_LUA_FUNC int alien_load(lua_State *L);
ALIEN_LUA_FUNC int alien_functionlist(lua_State *L);
ALIEN_LUA_FUNC int alien_hasfunction(lua_State *L);

alien_Library* alien_library__get_default_library(lua_State *L);
alien_Library* alien_library__get_misc(lua_State *L);

#endif // _ALIEN_LIBRARY_H_
