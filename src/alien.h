/* Lua FFI using libffi */
/* Author: Fabio Mascarenhas */
/* License: MIT/X11 */


#include "config.h"

#ifdef WIN32  
# ifndef WINDOWS
#  define WINDOWS
# endif
#endif

#ifdef WINDOWS
#define __EXPORT __declspec(dllexport)
#else
#define __EXPORT
#endif

#ifdef WINDOWS
#define _CRT_SECURE_NO_DEPRECATE 1
#include <windows.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# error "cannot find alloca"
#endif

#include "alien_dep_lua.h"

#define ALIEN_LUA_FUNC

