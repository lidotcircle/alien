#include <lua.hpp>
#include "alien_exception.h"

void* luaL_testudata(lua_State* L, int idx, const char* name);


void* luaL_checkudata_ex(lua_State* L, int idx, const char* name) {
    void* ans = nullptr;
    if (!(ans = luaL_testudata(L, idx, name)))
        throw AlienInvalidValueException("expect '%s' userdata", name);

    return ans;
}

const char* luaL_checkstring_ex(lua_State* L, int idx) {
    if (!lua_isstring(L, idx))
        throw AlienInvalidValueException("expect string");

    return luaL_checkstring(L, idx);
}

lua_Number  luaL_checkinteger_ex(lua_State* L, int idx) {
    if (!lua_isinteger(L, idx))
        throw AlienInvalidValueException("expect integer");

    return luaL_checkinteger(L, idx);
}

lua_Number  luaL_checknumber_ex(lua_State* L, int idx) {
    if (!lua_isnumber(L, idx))
        throw AlienInvalidValueException("expect number");

    return luaL_checknumber(L, idx);
}

