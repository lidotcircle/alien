#ifndef _ALIEN_TYPE_H_
#define _ALIEN_TYPE_H_

#include <lua.hpp>
#include <ffi.h>


int alien_types_init(lua_State* L);
int alien_types_register_basic(lua_State* L, const char* tname, ffi_type* ffitype);

int alien_types_defstruct(lua_State* L);
int alien_types_defunion(lua_State* L);
int alien_types_alias(lua_State* L);

#endif // _ALIEN_TYPE_H_
