#ifndef _ALIEN_DEP_LUA_H_
#define _ALIEN_DEP_LUA_H_

/* Lua 5.1 compatibility for Lua 5.2 */
#define LUA_COMPAT_ALL
/* Lua 5.2 compatibility for Lua 5.3 */
#define LUA_COMPAT_5_2

#if defined(linux)
#define PLATFORM "linux"
#define USE_DLOPEN
#elif defined(BSD)
#define PLATFORM "bsd"
#define USE_DLOPEN
#elif defined(__APPLE__)
#define PLATFORM "darwin"
#define USE_DLOPEN
#elif defined(WINDOWS)
#define PLATFORM "windows"
#else
#define PLATFORM "unknown"
#endif

#include <lua.hpp>

/* Lua 5.1 compatibility for Lua 5.3 */
#if LUA_VERSION_NUM == 503
#define lua_objlen(L,i)		(lua_rawlen(L, (i)))
#define luaL_register(L,n,l)	(luaL_newlib(L,l))
#endif

#if LUA_VERSION_NUM >= 502
int luaL_typerror (lua_State *L, int narg, const char *tname);
#else
#define lua_setuservalue lua_setfenv
#define lua_getuservalue lua_getfenv

void *luaL_testudata(lua_State *L, int ud, const char *tname);
#endif

#define LALLOC_FREE_STRING(lalloc, aud, s)              \
  (lalloc)((aud), (s), sizeof(char) * (strlen(s) + 1), 0)

void*       luaL_checkudata_ex(lua_State* L, int idx, const char* name);
const char* luaL_checkstring_ex(lua_State* L, int idx);
lua_Number  luaL_checkinteger_ex(lua_State* L, int idx);
lua_Number  luaL_checknumber_ex(lua_State* L, int idx);

#ifdef luaL_checkudata
#undef luaL_checkudata
#endif
#define luaL_checkudata(L, idx, name) luaL_checkudata_ex(L, idx, name)

#ifdef luaL_checkstring
#undef luaL_checkstring
#endif
#define luaL_checkstring(L, idx) luaL_checkstring_ex(L, idx)

#ifdef luaL_checkinteger
#undef luaL_checkinteger
#endif
#define luaL_checkinteger(L, idx) luaL_checkinteger_ex(L, idx)

#ifdef luaL_checknumber
#undef luaL_checknumber
#endif
#define luaL_checknumber(L, idx) luaL_checknumber_ex(L, idx)

#endif // _ALIEN_DEP_LUA_H_
