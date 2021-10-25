#include "alien.h"
#include <funchook.h>


int hook_function_table = 0;
static int get_hook_function_table(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, hook_function_table);
    return 1;
}

static int get_hook_entry(lua_State* L, const char* lib, const char* name) {
    void* aud = NULL;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    size_t a = 0, b = 0;
    if (lib != NULL)
        a = strlen(lib);
    if (name != NULL)
        b = strlen(name);
    char* ename = (char*)lalloc(aud, NULL, 0, a + b + 2);
    if (ename == NULL)
        return luaL_error(L, "aline: out of memory");
    if (a > 0)
        memcpy(ename, lib, a);
    ename[a] = '/';
    if (b > 0)
        strcpy(ename + a + 1, name);
    lua_pushstring(L, ename);
    printf("name = %s\n", ename);
    lalloc(aud, NULL, a + b + 2, 0);

    return 1;
}

int alien_function_hook(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    if (oac->trampoline_fn) {
        return luaL_error(L, "aline: function already be hooked");
    }
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushcfunction(L, alien_callback_new);
    lua_pushvalue(L, 2);
    lua_call(L, 1, 1);
    alien_Function* ac = alien_checkfunction(L, -1);

    lua_pushcfunction(L, alien_function_types);
    lua_pushvalue(L, -2);
    lua_pushnumber(L, oac->type_ref);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_call(L, 2, 0);

    funchook_t* hook = funchook_create();
    oac->trampoline_fn = oac->fn;
    oac->hookhandle = hook;
    if (funchook_prepare(hook, &oac->trampoline_fn, ac->ffi_codeloc) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: prepare hook failed");
    }
    if (funchook_install(hook, 0) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: install hook failed");
    }

/*
    luaL_ref(L, LUA_REGISTRYINDEX);
    */
    const char* libname = NULL;
    if (oac->lib != NULL)
        libname = oac->lib->name;
    get_hook_function_table(L);
    get_hook_entry(L, libname, oac->name);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 1;
}

int alien_function_horigin(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    if (oac->trampoline_fn == NULL)
        return luaL_error(L, "alien: unhook");

    lua_pushcfunction(L, alien_function_new);
    lua_pushlightuserdata(L, oac->trampoline_fn);
    lua_call(L, 1, 1);

    lua_pushcfunction(L, alien_function_types);
    lua_pushvalue(L, -2);
    lua_pushnumber(L, oac->type_ref);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_call(L, 2, 0);

    return 1;
}

int alien_function_unhook(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    if (oac->trampoline_fn == NULL) {
        return luaL_error(L, "aline: function hasn't hooked");
    }

    if (funchook_uninstall(static_cast<funchook_t*>(oac->hookhandle), 0) != FUNCHOOK_ERROR_SUCCESS) {
        return luaL_error(L, "aline: unhook failed");
    }
    funchook_destroy(static_cast<funchook_t*>(oac->hookhandle));
    oac->trampoline_fn = NULL;
    oac->hookhandle = NULL;

    return 0;
}

