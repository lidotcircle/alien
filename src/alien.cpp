#include <assert.h>
#include <string>
// #include <iostream> cause leak in valgrind
#include <ostream>
#include <istream>
#include <sstream>
#include <codecvt>
#include <locale>
#include <lua.hpp>
#include <algorithm>
#include <vector>
#include <map>
#include <math.h>
#include <set>
#include <memory>
#include <stdarg.h>
#include <stdexcept>
#include <tuple>
#include <stdio.h>
#include <fstream>

class 
//__attribute__((visibility("hidden"))) compile options -fvisibility=hidden
CL
{
    public:
        int cln();
        int xx() { throw std::runtime_error("xx"); }
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

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring str = converter.from_bytes("hello");

    return CL().cln();
}

