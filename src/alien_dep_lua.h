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

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

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

#endif // _ALIEN_DEP_LUA_H_
