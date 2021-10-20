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
        b = strlen(lib);
    char* ename = lalloc(aud, NULL, 0, a + b + 2);
    if (ename == NULL)
        return luaL_error(L, "aline: out of memory");
    if (a > 0)
        memcpy(ename, lib, a);
    ename[a] = '/';
    if (b > 0)
        strcpy(ename + a + 1, name);
    lua_pushstring(L, ename);
    lalloc(aud, NULL, a + b + 2, 0);

    return 1;
}

int alien_function_hook(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    if (oac->hookhandle) {
        return luaL_error(L, "aline: can't hook horigin function");
    }
    if (oac->is_hooked) {
        return luaL_error(L, "aline: function already be hooked");
    }

    luaL_checktype(L, 2, LUA_TFUNCTION);

    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    alien_Function *ac;
    ac = (alien_Function *)lua_newuserdata(L, sizeof(alien_Function));
    if(!ac) return luaL_error(L, "alien: out of memory");
    ac->fn = ffi_closure_alloc(sizeof(ffi_closure), &ac->ffi_codeloc);
    if(ac->fn == NULL) return luaL_error(L, "alien: cannot allocate callback");
    ac->L = L;
    ac->is_hooked = 0;
    ac->hookhandle = NULL;
    ac->lib = NULL;
    ac->name = NULL;
    ac->ret_type = oac->ret_type;
    ac->ffi_ret_type = oac->ffi_ret_type;
    ac->nparams = oac->nparams;
    if(ac->nparams > 0) {
        ac->ffi_params = (ffi_type **)lalloc(aud, NULL, 0, sizeof(ffi_type *) * ac->nparams);
        if(!ac->ffi_params) return luaL_error(L, "alien: out of memory");
        ac->params = (alien_Type *)lalloc(aud, NULL, 0, ac->nparams * sizeof(alien_Type));
        if(!ac->params) return luaL_error(L, "alien: out of memory");

        for (size_t i=0;i<ac->nparams;i++) {
            ac->ffi_params[i] = oac->ffi_params[i];
            ac->params[i] = oac->params[i];
        }
    } else {
        ac->ffi_params = NULL;
        ac->params = NULL;
    }
    lua_pushvalue(L, 2);
    ac->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, ALIEN_FUNCTION_META);
    lua_setmetatable(L, -2);

    ffi_status status = ffi_prep_cif(&(ac->cif), oac->cif.abi, ac->nparams,
            ac->ffi_ret_type, ac->ffi_params);
    if(status == FFI_OK)
        status = ffi_prep_closure_loc(ac->fn, &(ac->cif), &alien_callback_call, ac, ac->ffi_codeloc);
    if(status != FFI_OK) {
        ffi_closure_free(ac->fn);
        return luaL_error(L, "alien: cannot create callback");
    }

    funchook_t* hook = funchook_create();
    ac->scc = ac->fn;
    ac->fn = oac->fn;
    if (funchook_prepare(hook, &ac->fn, ac->ffi_codeloc) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: prepare hook failed");
    }
    if (funchook_install(hook, 0) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: install hook failed");
    }
    ac->hookhandle = hook;
    oac->is_hooked = 1;

    ac->lib = oac->lib;
    const char* libname = NULL;
    if (oac->lib != NULL)
        libname = oac->lib->name;
    const char* horigin_prefix = "horigin_";
    size_t namelen = 0;
    if (oac->name != NULL)
        namelen = strlen(oac->name);
    ac->name = lalloc(aud, NULL, 0, strlen(horigin_prefix) + namelen + 1);
    if (ac->name == NULL)
        return luaL_error(L, "aline: out of memory");
    strcpy(ac->name, horigin_prefix);
    if (namelen > 0)
        strcpy(ac->name + strlen(horigin_prefix), oac->name);

    get_hook_function_table(L);
    get_hook_entry(L, libname, oac->name);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    return 1;
}

int alien_function_horigin(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    get_hook_function_table(L);
    const char* libname = NULL;
    if (oac->lib != NULL)
        libname = oac->lib->name;
    get_hook_entry(L, libname, oac->name);
    lua_gettable(L, -2);

    return 1;
}

int alien_function_unhook(lua_State* L) {
    alien_Function* oac = alien_checkfunction(L, 1);
    if (!oac->is_hooked) {
        return luaL_error(L, "aline: function hasn't hooked");
    }

    get_hook_function_table(L);
    const char* libname = NULL;
    if (oac->lib != NULL)
        libname = oac->lib->name;
    get_hook_entry(L, libname, oac->name);
    lua_gettable(L, -2);

    if (lua_isnil(L, -1)) {
        return luaL_error(L, "aline: function desn't be hooked");
    }

    alien_Function* ac = alien_checkfunction(L, -1);
    if (funchook_uninstall(ac->hookhandle, 0) != FUNCHOOK_ERROR_SUCCESS) {
        return luaL_error(L, "aline: unhook failed");
    }
    funchook_destroy(ac->hookhandle);
    oac->is_hooked = 0;
    ac->hookhandle = NULL;
    ac->fn = ac->scc;
    ac->scc = NULL;
    lua_pop(L, 1);

    get_hook_entry(L, libname, oac->name);
    lua_pushnil(L);
    lua_settable(L, -3);

    return 0;
}

