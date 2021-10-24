#include "alien.h"


const char *const alien_typenames[] =  {
#define MENTRY(_n, _b, _s, _a) (_n),
    type_map
#undef MENTRY
    NULL
};

#define ffi_type_byte      ffi_type_sint8
#define ffi_type_char      ffi_type_uchar
#define ffi_type_short     ffi_type_sshort
#define ffi_type_int       ffi_type_sint
#define ffi_type_long      ffi_type_slong
#define ffi_type_string    ffi_type_pointer
#define ffi_type_refchar   ffi_type_pointer
#define ffi_type_refint    ffi_type_pointer
#define ffi_type_refuint   ffi_type_pointer
#define ffi_type_refdouble ffi_type_pointer
#define ffi_type_longlong  ffi_type_sint64
#define ffi_type_ulonglong ffi_type_uint64
#define ffi_type_callback  ffi_type_pointer


#ifdef WINDOWS

ffi_type* ffitypes[AT_ENTRY_COUNT];

#else

ffi_type* ffitypes[] = {
#define MENTRY(_n, _b, _s, _a) &ALIEN_SPLICE(ffi_type_, _b),
    type_map
#undef MENTRY
        NULL
};

#endif

/******************************************************************/

alien_Library *alien_checklibrary(lua_State *L, int index) {
    return (alien_Library *)luaL_checkudata(L, index, ALIEN_LIBRARY_META);
}

alien_Function *alien_checkfunction(lua_State *L, int index) {
    return (alien_Function *)luaL_checkudata(L, index, ALIEN_FUNCTION_META);
}

alien_Buffer *alien_checkbuffer(lua_State *L, int index) {
    return (alien_Buffer *)luaL_checkudata(L, index, ALIEN_BUFFER_META);
}

void *alien_touserdata(lua_State *L, int index) {
    void *ud = lua_touserdata(L, index);
    if(!ud) return NULL;
    return luaL_testudata(L, index, ALIEN_BUFFER_META) ? ((alien_Buffer *)ud)->p : ud;
}

void *alien_checknonnull(lua_State *L, int index) {
    void *p = alien_touserdata(L, index);
    if(!p) {
        /* Calls to luaL_typerror cannot be preceded by "return" as it's
           the wrong type here. */
        if (lua_isuserdata(L, index))
            luaL_typerror(L, index, "non-NULL argument");
        luaL_typerror(L, 1, "userdata");
    }
    return p;
}

static int alien_sizeof(lua_State *L) {
    static const size_t sizes[] = {
#define MENTRY(_n, _b, _s, _a)  sizeof(_s),
        type_map
#undef MENTRY
            0
    };
    lua_pushinteger(L, sizes[luaL_checkoption(L, 1, "int", alien_typenames)]);
    return 1;
}

static int alien_align(lua_State *L) {
    static const size_t aligns[] = {
        0
#define MENTRY(_n, _b, _s, _a) ALIEN_SPLICE(_a, _ALIGN),
            type_map
#undef MENTRY
            0
    };
    lua_pushinteger(L, aligns[luaL_checkoption(L, 1, "char", alien_typenames)]);
    return 1;
}

static int alien_register(lua_State *L) {
    const char *meta = luaL_checkstring(L, 1);
    luaL_getmetatable(L, meta);
    if(lua_isnil(L, -1))
        luaL_newmetatable(L, meta);
    return 1;
}

static void alien_wrap_one(lua_State *L, int i, alien_Wrap *ud) {
    if(lua_isnoneornil(L, i)) {
        ud->tag = AT_pointer;
        ud->val.p = NULL;
    } else if(lua_isuserdata(L, i)) {
        ud->tag = AT_pointer;
        ud->val.p = alien_touserdata(L, i);
    } else {
        ud->tag = AT_int;
        ud->val.i = lua_tointeger(L, i);
    }
}

static int alien_wrap(lua_State *L) {
    const char *meta = luaL_checkstring(L, 1);
    alien_Wrap *ud = (alien_Wrap*)lua_newuserdata(L, sizeof(alien_Wrap) * lua_gettop(L));
    int top = lua_gettop(L);
    int i;
    for(i = 2; i < top; i++)
        alien_wrap_one(L, i, ud++);
    ud->tag = AT_void;
    luaL_getmetatable(L, meta);
    lua_setmetatable(L, -2);
    return 1;
}

static int alien_rewrap(lua_State *L) {
    const char *meta = luaL_checkstring(L, 1);
    alien_Wrap *ud = (alien_Wrap *)luaL_checkudata(L, 2, meta);
    int i;
    for (i = 3; ud->tag != AT_void; i++)
        alien_wrap_one(L, i, ud++);
    return 0;
}

static int alien_unwrap(lua_State *L) {
    const char *meta = luaL_checkstring(L, 1);
    alien_Wrap *ud = (alien_Wrap *)luaL_checkudata(L, 2, meta);
    while(ud->tag != AT_void) {
        switch(ud->tag) {
            case AT_int: lua_pushnumber(L, ud->val.i); break;
            case AT_pointer: ud->val.p ? lua_pushlightuserdata(L, ud->val.p) :
                             lua_pushnil(L); break;
            default: return luaL_error(L, "alien: wrong type in wrapped value");
        }
        ud++;
    }
    return lua_gettop(L) - 2;
}

#define alien_udata2x_head(name, type, ptrtype) \
    static int alien_udata2 ## name(lua_State *L) { \
        type *ud; \
        size_t size; \
        if(lua_isnil(L, 1)) { \
            lua_pushnil(L); \
            return 1; \
        } \
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA); \
        ud = (ptrtype)lua_touserdata(L, 1); \
        if(lua_gettop(L) >= 2 && !lua_isnil(L, 2)) \
        size = luaL_checkinteger(L, 2); \
        else

#define alien_udata2x(name, type) \
        alien_udata2x_head(name, type, type *) \
        size = 1; \
        { \
            size_t i; \
            for (i = 0; i < size; i++) \
            lua_pushnumber(L, ud[i]); \
        } \
        return size; \
    }

/* Define alien_udata2str as a special case */
alien_udata2x_head(str, char, char *)
    size = strlen(ud);
    lua_pushlstring(L, ud, size);
    return 1;
    }

    alien_udata2x(char, char)
    alien_udata2x(short, short)
    alien_udata2x(ushort, unsigned short)
    alien_udata2x(int, int)
    alien_udata2x(uint, unsigned int)
    alien_udata2x(long, long)
    alien_udata2x(ulong, unsigned long)
    alien_udata2x(ptrdiff_t, ptrdiff_t)
    alien_udata2x(size_t, size_t)
    alien_udata2x(float, float)
alien_udata2x(double, double)

    static int alien_isnull(lua_State *L) {
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        lua_pushboolean(L, lua_touserdata(L, 1) == NULL);
        return 1;
    }


/* Expose existing APIs for convenience */

static int alien_table_new(lua_State *L) {
    int narray = luaL_optint(L, 1, 0);
    int nhash = luaL_optint(L, 2, 0);
    lua_createtable(L, narray, nhash);
    return 1;
}

static int alien_errno(lua_State *L) {
    lua_pushnumber(L, errno);
    return 1;
}

static int alien_memmove(lua_State *L) {
    void* src;
    size_t size;
    void *dst = alien_checknonnull(L, 1);
    if (!(lua_isuserdata(L, 2) || lua_isstring(L, 2)))
        return luaL_typerror(L, 2, "string or userdata");
    if (lua_isuserdata(L, 2)) {
        src = alien_checknonnull(L, 2);
        size = luaL_checkinteger(L, 3);
    } else {
        src = (void*)lua_tolstring(L, 2, &size);
        size = luaL_optinteger(L, 3, size);
    }
    if (size > 0)
        memmove(dst, src, size);
    return 0;
}

static int alien_memset(lua_State *L) {
    int c;
    size_t n;
    void *dst = alien_touserdata(L, 1);
    if(!dst)
        return luaL_typerror(L, 1, "userdata");
    c = luaL_checkinteger(L, 2);
    n = luaL_checkinteger(L, 3);
    memset(dst, c, n);
    return 0;
}

static const luaL_Reg alienlib[] = {
    {"load", alien_load},
    {"functionlist", alien_functionlist},
    {"hasfunction", alien_hasfunction},
    {"align", alien_align},
    {"tag", alien_register},
    {"wrap", alien_wrap},
    {"rewrap", alien_rewrap},
    {"unwrap", alien_unwrap},
    {"tostring", alien_udata2str},
    {"isnull", alien_isnull},
    {"sizeof", alien_sizeof},
    {"tochar", alien_udata2char},
    {"toshort", alien_udata2short},
    {"toushort", alien_udata2ushort},
    {"toint", alien_udata2int},
    {"touint", alien_udata2uint},
    {"tolong", alien_udata2long},
    {"toulong", alien_udata2ulong},
    {"toptrdiff_t", alien_udata2ptrdiff_t},
    {"tosize_t", alien_udata2size_t},
    {"tofloat", alien_udata2float},
    {"todouble", alien_udata2double},
    {"buffer", alien_buffer_new},
    {"callback", alien_callback_new},
    {"funcptr", alien_function_new},
    {"table", alien_table_new},
    {"errno", alien_errno},
    {"memmove", alien_memmove },
    {"memset", alien_memset },

    /* Struct functions */
    {"pack", b_pack},
    {"unpack", b_unpack},
    {"size", b_size},
    {"offset", b_offset},

    {NULL, NULL},
};

__EXPORT int luaopen_alien_c(lua_State *L) {
    alien_Library *al;

#ifdef WINDOWS

    int i = 0;
#define MENTRY(_n, _b, _s, _a) { ffitypes[i] = &ALIEN_SPLICE(ffi_type_, _b); i++; }
    type_map
#undef MENTRY

    ffitypes[i] = NULL;
#endif

    /* Library metatable */
    luaL_newmetatable(L, ALIEN_LIBRARY_META);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_library_gc);
    lua_settable(L, -3);
    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_library_tostring);
    lua_settable(L, -3);
    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_library_get);
    lua_settable(L, -3);
    lua_pop(L, 1);

    /* Function metatable */
    luaL_newmetatable(L, ALIEN_FUNCTION_META);
    lua_pushliteral(L, "__index");
    lua_newtable(L);
    lua_pushliteral(L, "types");
    lua_pushcfunction(L, alien_function_types);
    lua_settable(L, -3);
    lua_pushliteral(L, "addr");
    lua_pushcfunction(L, alien_function_addr);
    lua_settable(L, -3);
    lua_pushliteral(L, "hook");
    lua_pushcfunction(L, alien_function_hook);
    lua_settable(L, -3);
    lua_pushliteral(L, "unhook");
    lua_pushcfunction(L, alien_function_unhook);
    lua_settable(L, -3);
    lua_pushliteral(L, "horigin");
    lua_pushcfunction(L, alien_function_horigin);
    lua_settable(L, -3);
    lua_settable(L, -3);
    lua_pushliteral(L, "__call");
    lua_pushcfunction(L, alien_function_call);
    lua_settable(L, -3);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_function_gc);
    lua_settable(L, -3);
    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_function_tostring);
    lua_settable(L, -3);
    lua_pop(L, 1);

    /* Buffer metatable */
    luaL_newmetatable(L, ALIEN_BUFFER_META);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_buffer_gc);
    lua_settable(L, -3);
    lua_pushliteral(L, "__len");
    lua_pushcfunction(L, alien_buffer_length);
    lua_settable(L, -3);
    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_buffer_get);
    lua_settable(L, -3);
    lua_pushliteral(L, "__newindex");
    lua_pushcfunction(L, alien_buffer_set);
    lua_settable(L, -3);
    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_buffer_tostring);
    lua_settable(L, -3);
    lua_pop(L, 1);

    /* library function cache */
    lua_newtable(L);
    library_cache_entry = luaL_ref(L, LUA_REGISTRYINDEX);

    /* hook function table */
    lua_newtable(L);
    hook_function_table = luaL_ref(L, LUA_REGISTRYINDEX);

    /* callback thread mutex */
    lua_pushlightuserdata(L, NULL);
    lua_setglobal(L, "callback_mutex");

    /* Register main library */
    luaL_register(L, "alien", alienlib);
    /* Version */
    lua_pushliteral(L, MYVERSION);
    lua_setfield(L, -2, "version");
    /* Set platform */
    lua_pushliteral(L, PLATFORM);
    lua_setfield(L, -2, "platform");
    /* Initialize libraries table */
    al = (alien_Library *)lua_newuserdata(L, sizeof(alien_Library));
    al->lib = NULL;
    al->name = "default";
    lua_newtable(L);
    lua_setuservalue(L, -2);
    luaL_getmetatable(L, ALIEN_LIBRARY_META);
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "default");

    return 1;
}
