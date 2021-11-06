#include <assert.h>
#include <string>
// #include <iostream> cause leak in valgrind
#include <lua.hpp>

class 
//__attribute__((visibility("hidden"))) compile options -fvisibility=hidden
CL
{
    public:
        int cln();
};

int CL::cln() {return 1;}

static const luaL_Reg alienlib[] = {
    {NULL, NULL},
};

extern "C" 
__attribute__((visibility("default")))
int luaopen_alien_c(lua_State *L) {
    /* Register main library */
    luaL_newlib(L, alienlib);

    return CL().cln();
}

