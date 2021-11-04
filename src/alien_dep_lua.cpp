#include "alien.h"
#include "alien_exception.h"


#if LUA_VERSION_NUM >= 502
int luaL_typerror (lua_State *L, int narg, const char *tname) {
    const char *msg = lua_pushfstring(L, "%s expected, got %s",
            tname, luaL_typename(L, narg));
    return luaL_argerror(L, narg, msg);
}
#else
void *luaL_testudata(lua_State *L, int ud, const char *tname) {
    void *p = lua_touserdata(L, ud);
    if(p == NULL) return NULL;
    if(!lua_getmetatable(L, ud)) return NULL;
    lua_getfield(L, LUA_REGISTRYINDEX, tname);
    if(!lua_rawequal(L, -1, -2))
        p = NULL;
    lua_pop(L, 2);
    return p;
}
#endif

