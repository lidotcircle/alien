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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include <ffi.h>

/* libffi extension to support size_t and ptrdiff_t */
#if PTRDIFF_MAX == 65535
# define ffi_type_size_t         ffi_type_uint16
# define ffi_type_ptrdiff_t      ffi_type_sint16
#elif PTRDIFF_MAX == 2147483647
# define ffi_type_size_t         ffi_type_uint32
# define ffi_type_ptrdiff_t      ffi_type_sint32
#elif PTRDIFF_MAX == 9223372036854775807
# define ffi_type_size_t         ffi_type_uint64
# define ffi_type_ptrdiff_t      ffi_type_sint64
#elif defined(_WIN64)
# define ffi_type_size_t         ffi_type_uint64
# define ffi_type_ptrdiff_t      ffi_type_sint64
#elif defined(WINDOWS)
# define ffi_type_size_t         ffi_type_uint32
# define ffi_type_ptrdiff_t      ffi_type_sint32
#else
 #error "ptrdiff_t size not supported"
#endif


#include "alien_dep_lua.h"

#define MYNAME          "alien"
#define MYVERSION       MYNAME " library for " LUA_VERSION " / " VERSION
#define FUNCTION_CACHE  "__library_functions_cache"

/* The extra indirection to these macros is required so that if the
   arguments are themselves macros, they will get expanded too.  */
#define ALIEN__SPLICE(_s, _t)   _s##_t
#define ALIEN_SPLICE(_s, _t)    ALIEN__SPLICE(_s, _t)

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

#define ALIEN_LIBRARY_META  "alien library"
#define ALIEN_FUNCTION_META "alien function"
#define ALIEN_BUFFER_META   "alien buffer"

/* Information to compute structure access */

typedef struct { char c; char x; } s_char;
typedef struct { char c; short x; } s_short;
typedef struct { char c; int x; } s_int;
typedef struct { char c; long x; } s_long;
typedef struct { char c; ptrdiff_t x; } s_ptrdiff_t_p;
typedef struct { char c; long long x; } s_longlong;
typedef struct { char c; float x; } s_float;
typedef struct { char c; double x; } s_double;
typedef struct { char c; char *x; } s_char_p;
typedef struct { char c; void *x; } s_void_p;

#define AT_NONE_ALIGN /* unused */
#define AT_CHAR_ALIGN (offsetof(s_char, x))
#define AT_SHORT_ALIGN (offsetof(s_short, x))
#define AT_INT_ALIGN (offsetof(s_int, x))
#define AT_LONG_ALIGN (offsetof(s_long, x))
#define AT_PTRDIFF_T_P_ALIGN (offsetof(s_ptrdiff_t_p, x))
#define AT_LONGLONG_ALIGN (offsetof(s_longlong, x))
#define AT_FLOAT_ALIGN (offsetof(s_float, x))
#define AT_DOUBLE_ALIGN (offsetof(s_double, x))
#define AT_CHAR_P_ALIGN (offsetof(s_char_p, x))
#define AT_VOID_P_ALIGN (offsetof(s_void_p, x))

/*              NAME          BASE       SIZEOF                 ALIGNMENT
                ====          ====       ======                 =========       */
#define type_map \
        MENTRY( "void",       void,      char,                  AT_NONE         ) \
        MENTRY( "byte",       byte,      unsigned char,         AT_CHAR         ) \
        MENTRY( "char",       char,      char,                  AT_CHAR         ) \
        MENTRY( "short",      short,     short,                 AT_SHORT        ) \
        MENTRY( "ushort",     ushort,    unsigned short,        AT_SHORT        ) \
        MENTRY( "int",        int,       int,                   AT_INT          ) \
        MENTRY( "uint",       uint,      unsigned int,          AT_INT          ) \
        MENTRY( "long",       long,      long,                  AT_LONG         ) \
        MENTRY( "ulong",      ulong,     unsigned long,         AT_LONG         ) \
        MENTRY( "ptrdiff_t",  ptrdiff_t, ptrdiff_t,             AT_PTRDIFF_T_P  ) \
        MENTRY( "size_t",     size_t,    size_t,                AT_PTRDIFF_T_P  ) \
        MENTRY( "float",      float,     float,                 AT_FLOAT        ) \
        MENTRY( "double",     double,    double,                AT_DOUBLE       ) \
        MENTRY( "string",     string,    char *,                AT_CHAR_P       ) \
        MENTRY( "pointer",    pointer,   void *,                AT_VOID_P       ) \
        MENTRY( "ref char",   refchar,   char *,                AT_CHAR_P       ) \
        MENTRY( "ref int",    refint,    int *,                 AT_VOID_P       ) \
        MENTRY( "ref uint",   refuint,   unsigned int *,        AT_VOID_P       ) \
        MENTRY( "ref double", refdouble, double *,              AT_VOID_P       ) \
        MENTRY( "longlong",   longlong,  long long,             AT_LONGLONG     ) \
        MENTRY( "ulonglong",  ulonglong, unsigned long long,    AT_LONGLONG     ) \
        MENTRY( "callback",   callback,  void *,                AT_VOID_P       )

typedef enum {
#define MENTRY(_n, _b, _s, _a) ALIEN_SPLICE(AT_, _b),
  type_map
#undef MENTRY
  AT_ENTRY_COUNT
} alien_Type;

extern const char *const alien_typenames[];
extern ffi_type* ffitypes[];

extern int library_cache_entry;
extern int hook_function_table;

typedef struct {
  void *lib;
  char *name;
} alien_Library;

typedef struct {
  alien_Library *lib;
  void *fn;
  char *name;
  alien_Type ret_type;
  ffi_cif cif;
  ffi_type *ffi_ret_type;
  int nparams;
  alien_Type *params;
  ffi_type **ffi_params;

  int type_ref;
  /* hook part */
  void* hookhandle;
  void* trampoline_fn;

  /* callback part */
  lua_State *L;
  void *ffi_codeloc;
  int fn_ref;
} alien_Function;

typedef struct {
  char *p;
  size_t size;
} alien_Buffer;

typedef struct {
  alien_Type tag;
  union {
    void *p;
    int i;
  } val;
} alien_Wrap;

alien_Library  *alien_checklibrary(lua_State *L, int index);
alien_Function *alien_checkfunction(lua_State *L, int index);
alien_Buffer   *alien_checkbuffer(lua_State *L, int index);
void           *alien_touserdata(lua_State *L, int index);
void           *alien_checknonnull(lua_State *L, int index);

int alien_load(lua_State *L);
int alien_functionlist(lua_State *L);
int alien_hasfunction(lua_State *L);
int alien_library_get(lua_State *L);
int alien_library_tostring(lua_State *L);
int alien_library_gc(lua_State *L);

int alien_makefunction(lua_State *L, void *lib, void *fn, char *name);
int alien_function_new(lua_State *L);
int alien_function_gc(lua_State *L);
int alien_function_types(lua_State *L);
int alien_function_tostring(lua_State *L);
int alien_function_call(lua_State *L);
int alien_function_addr(lua_State *L);

int  alien_callback_new(lua_State *L);
void alien_callback_call(ffi_cif *cif, void *resp, void **args, void *data);

int alien_function_hook(lua_State* L);
int alien_function_unhook(lua_State* L);
int alien_function_horigin(lua_State* L);

int alien_buffer_new(lua_State *L);
int alien_buffer_gc(lua_State *L);
int alien_buffer_length(lua_State *L);
int alien_buffer_tostring(lua_State *L);
int alien_buffer_strlen(lua_State *L);
int alien_buffer_topointer(lua_State *L);
int alien_buffer_tooffset(lua_State *L);
int alien_buffer_set(lua_State *L);
int alien_buffer_realloc(lua_State *L);
int alien_buffer_get(lua_State *L);

int b_pack (lua_State *L);
int b_unpack (lua_State *L);
int b_size (lua_State *L);
int b_offset (lua_State *L);
