#ifndef _ALIEN_LUA_UTIL_H_
#define _ALIEN_LUA_UTIL_H_

#include "alien_dep_lua.h"
#include "alien.h"


int  alien_rotable_init(lua_State* L);

ALIEN_LUA_FUNC int  alien_rotable_new(lua_State* L);
bool alien_isrotable(lua_State* L, int idx);
int  alien_rotable_rawtable(lua_State* L, int idx);

#endif // _ALIEN_LUA_UTIL_H_
