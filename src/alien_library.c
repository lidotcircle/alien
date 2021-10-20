#include "alien.h"
#include "utils.h"


#if defined(USE_DLOPEN)

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT 0
#endif

#include <dlfcn.h>

static void alien_unload (void *lib) {
    if(lib && (lib != RTLD_DEFAULT))
        dlclose(lib);
}

static void *alien_openlib (lua_State *L, const char *libname) {
    void *lib = dlopen(libname, RTLD_NOW);
    if(lib == NULL) lua_pushstring(L, dlerror());
    return lib;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
    if(!lib) lib = RTLD_DEFAULT;
    void *f = dlsym(lib, sym);
    if (f == NULL) lua_pushstring(L, dlerror());
    return f;
}

#elif defined(WINDOWS)

#define PLATFORM "windows"

static void pusherror (lua_State *L) {
    int error = GetLastError();
    char buffer[128];
    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, error, 0, buffer, sizeof(buffer), NULL))
        lua_pushstring(L, buffer);
    else
        lua_pushfstring(L, "system error %d\n", error);
}

static void alien_unload (void *lib) {
    if(lib)
        FreeLibrary((HINSTANCE)lib);
}

static void *alien_openlib (lua_State *L, const char *libname) {
    HINSTANCE lib = LoadLibrary(libname);
    if (lib == NULL) pusherror(L);
    return lib;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
    HINSTANCE module;
    void *f;
    module = (HINSTANCE)lib;
    if(!module) module = GetModuleHandle(NULL);
    f = (lua_CFunction)GetProcAddress(module, sym);
    if (f == NULL) pusherror(L);
    return f;
}

#else

#define DLMSG   "dynamic libraries not enabled; check your Lua installation"

#define PLATFORM "unknown"

static void alien_unload (void *lib) {
    (void)lib;  /* to avoid warnings */
}

static void *alien_openlib (lua_State *L, const char *path) {
    (void)path;  /* to avoid warnings */
    lua_pushliteral(L, DLMSG);
    return NULL;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
    (void)lib; (void)sym;  /* to avoid warnings */
    lua_pushliteral(L, DLMSG);
    return NULL;
}

#endif


int alien_load(lua_State *L) {
    size_t len;
    void *aud, *lib;
    alien_Library *al;
    const char *libname = luaL_checklstring(L, lua_gettop(L), &len);
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    char *name = (char*)lalloc(aud, NULL, 0, sizeof(char) * (len + 1));
    if(!name)
        return luaL_error(L, "alien: out of memory");
    strcpy(name, libname);
    al = (alien_Library *)lua_newuserdata(L, sizeof(alien_Library));
    if(!al) return luaL_error(L, "alien: out of memory");
    lib = alien_openlib(L, libname);
    if(!lib)
        return lua_error(L);
    lua_newtable(L);
    lua_setuservalue(L, -2);
    luaL_getmetatable(L, ALIEN_LIBRARY_META);
    lua_setmetatable(L, -2);
    al->lib = lib;
    al->name = name;
    return 1;
}

int library_cache_entry = 0;
static int get_cache(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, library_cache_entry);
    return 1;
}

int alien_functionlist(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1)
        return luaL_error(L, "alien: too %s arguments[ functionlist(alien_library) ]", 
                n > 1 ? "many" : "few");

    alien_Library* al = alien_checklibrary(L, 1);
    get_cache(L);
    lua_getfield(L, -1, al->name);
    if (!lua_isnil(L, -1))
        return 1;
    lua_pop(L, 2);

    function_list* functions = lf_load(al->lib);
    if (functions == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    for (size_t i=1;i<lf_size(functions);i++) {
        lua_pushnumber(L, i);
        lua_pushstring(L, lf_index(functions, i - 1));
        lua_settable(L, -3);
    }

    lf_free(functions);

    get_cache(L);
    lua_setfield(L, -2, al->name);
    return 1;
}

int alien_hasfunction(lua_State* L) {
    size_t nargs = lua_gettop(L);
    if (nargs != 2) {
        return luaL_error(L, "alien: too %s arguments (function hasfunction)",
                nargs < 2 ? "few" : "many");
    }

    alien_Library* al = alien_checklibrary(L, 1);
    size_t len;
    const char* funcname = luaL_checklstring(L, 2, &len);

    lua_pushcfunction(L, alien_functionlist);
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);

    size_t n = lua_objlen(L, 3);
    for(size_t i=0;i<n;i++) {
        lua_geti(L, 3, i + 1);
        const char* u = luaL_checkstring(L, -1);

        if (!strcmp(u, funcname)) {
            lua_pushboolean(L, 1);
            return 1;
        }
        lua_pop(L, 1);
    }

    lua_pushboolean(L, 0);
    return 1;
}

int alien_library_get(lua_State *L) {
    char *name;
    int cache;
    void *aud, *fn;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    alien_Library *al = alien_checklibrary(L, 1);
    size_t len;
    const char *funcname = luaL_checklstring(L, 2, &len);
    lua_getuservalue(L, 1);
    cache = lua_gettop(L);
    lua_getfield(L, cache, funcname);
    if(!lua_isnil(L, -1)) return 1;
    name = (char*)lalloc(aud, NULL, 0, sizeof(char) * (len + 1));
    if(!name)
        return luaL_error(L, "alien: out of memory");
    strcpy(name, funcname);
    fn = alien_loadfunc(L, al->lib, funcname);
    if(!fn) {
        LALLOC_FREE_STRING(lalloc, aud, name);
        return lua_error(L);
    }
    alien_makefunction(L, al, fn, name);
    lua_newtable(L);
    lua_pushvalue(L, 1);
    lua_rawseti(L, -2, 1);
    lua_setuservalue(L, -2);
    lua_pushvalue(L, -1);
    lua_setfield(L, cache, funcname);
    return 1;
}

int alien_library_tostring(lua_State *L) {
    alien_Library *al = alien_checklibrary(L, 1);
    lua_pushfstring(L, "alien library %s", al->name);
    return 1;
}

int alien_library_gc(lua_State *L) {
    alien_Library *al = alien_checklibrary(L, 1);
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);

    if (al->name != NULL) {
        get_cache(L);
        lua_pushstring(L, al->name);
        lua_pushnil(L);
        lua_settable(L, -3);
        lua_pop(L, 1);
    }

    if(al->lib) {
        alien_unload(al->lib);
        al->lib = NULL;
        if(al->name) { LALLOC_FREE_STRING(lalloc, aud, al->name); al->name = NULL; }
    }
    return 0;
}

