#include "alien.h"
#include <funchook.h>


#if defined(X86_WIN32)
static const ffi_abi ffi_abis[] = { FFI_SYSV, FFI_STDCALL, FFI_THISCALL, FFI_FASTCALL, FFI_MS_CDECL, FFI_PASCAL, FFI_REGISTER, FFI_DEFAULT_ABI };
static const char *const ffi_abi_names[] = { "sysv", "stdcall", "thiscall", "fastcall", "cdecl", "pascal", "register", "default", NULL };
#elif defined(X86_WIN64)
static const ffi_abi ffi_abis[] = { FFI_WIN64, FFI_GNUW64, FFI_DEFAULT_ABI };
static const char *const ffi_abi_names[] = { "win64", "gnuw64", "default", NULL };
#elif defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
static const ffi_abi ffi_abis[] = { FFI_UNIX64, FFI_WIN64, FFI_EFI64, FFI_GNUW64, FFI_DEFAULT_ABI };
static const char *const ffi_abi_names[] = { "unix64", "win64", "efi64", "gnuw64", "default", NULL };
#else
static const ffi_abi ffi_abis[] = { FFI_SYSV, FFI_THISCALL, FFI_FASTCALL, FFI_STDCALL, FFI_PASCAL, FFI_REGISTER, FFI_MS_CDECL, FFI_DEFAULT_ABI };
static const char *const ffi_abi_names[] = { "sysv", "thiscall", "fastcall", "stdcall", "pascal", "register", "cdecl", "default", NULL };
#endif


int alien_makefunction(lua_State *L, void *lib, void *fn, char *name) {
    alien_Function *af = (alien_Function *)lua_newuserdata(L, sizeof(alien_Function));
    if(!af)
        return luaL_error(L, "alien: out of memory");
    luaL_getmetatable(L, ALIEN_FUNCTION_META);
    lua_setmetatable(L, -2);
    af->fn = fn;
    af->fn_ref = 0;
    af->name = name;
    af->lib = lib;
    af->nparams = 0;
    af->ret_type = AT_void;
    af->params = NULL;
    af->ffi_params = NULL;
    af->is_hooked = 0;
    af->hookhandle = NULL;
    return 1;
}

int alien_function_new(lua_State *L) {
    void *fn = lua_touserdata(L, 1);
    if(!fn)
        return luaL_error(L, "alien: not a userdata");
    return alien_makefunction(L, NULL, fn, NULL);
}

int alien_function_types(lua_State *L) {
    ffi_status status;
    ffi_abi abi;
    alien_Function *af = alien_checkfunction(L, 1);
    if (af->hookhandle) {
        return luaL_error(L, "aline: type of horigin function must be confirmed when creating");
    }
    if (af->is_hooked) {
        return luaL_error(L, "aline: can't change type of hooked function, TODO");
    }
    int i, ret_type;
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    if(lua_istable(L, 2)) {
        lua_getfield(L, 2, "ret");
        ret_type = luaL_checkoption(L, -1, "int", alien_typenames);
        af->ret_type = ret_type;
        af->ffi_ret_type = ffitypes[ret_type];
        lua_getfield(L, 2, "abi");
        abi = ffi_abis[luaL_checkoption(L, -1, "default", ffi_abi_names)];
        lua_pop(L, 2);
    } else {
        ret_type = luaL_checkoption(L, 2, "int", alien_typenames);
        af->ret_type = ret_type;
        af->ffi_ret_type = ffitypes[ret_type];
        abi = FFI_DEFAULT_ABI;
    }
    if(af->params) {
        lalloc(aud, af->params, sizeof(alien_Type) * af->nparams, 0);
        lalloc(aud, af->ffi_params, sizeof(ffi_type *) * af->nparams, 0);
        af->params = NULL; af->ffi_params = NULL;
    }
    af->nparams = lua_istable(L, 2) ? lua_objlen(L, 2) : lua_gettop(L) - 2;
    if(af->nparams > 0) {
        af->ffi_params = (ffi_type **)lalloc(aud, NULL, 0, sizeof(ffi_type *) * af->nparams);
        if(!af->ffi_params) return luaL_error(L, "alien: out of memory");
        af->params = (alien_Type *)lalloc(aud, NULL, 0, af->nparams * sizeof(alien_Type));
        if(!af->params) return luaL_error(L, "alien: out of memory");
    } else {
        af->ffi_params = NULL;
        af->params = NULL;
    }
    if(lua_istable(L, 2)) {
        for(i = 0; i < af->nparams; i++) {
            int type;
            lua_rawgeti(L, 2, i + 1);
            type = luaL_checkoption(L, -1, "int", alien_typenames);
            af->params[i] = type;
            af->ffi_params[i] = ffitypes[type];
            lua_pop(L, 1);
        }
    } else {
        for(i = 0; i < af->nparams; i++) {
            int type = luaL_checkoption(L, i + 3, "int", alien_typenames);
            af->ffi_params[i] = ffitypes[type];
            af->params[i] = type;
        }
    }
    status = ffi_prep_cif(&(af->cif), abi, af->nparams,
            af->ffi_ret_type,
            af->ffi_params);
    if(status != FFI_OK)
        return luaL_error(L, "alien: error in libffi preparation");
    if(af->fn_ref) {
        status = ffi_prep_closure_loc(af->fn, &(af->cif), &alien_callback_call, af, af->ffi_codeloc);
        if(status != FFI_OK) return luaL_error(L, "alien: cannot create callback");
    }
    return 0;
}

int alien_function_tostring(lua_State *L) {
    alien_Function *af = alien_checkfunction(L, 1);
    lua_pushfstring(L, "alien function %s, library %s", af->name ? af->name : "anonymous",
            ((af->lib && af->lib->name) ? af->lib->name : "default"));
    return 1;
}

int alien_function_call(lua_State *L) {
    int iret; double dret; void *pret; long lret; unsigned long ulret; float fret;
    int i, nrefi = 0, nrefui = 0, nrefd = 0, nrefc = 0;
    void **args = NULL;
    alien_Function *af = alien_checkfunction(L, 1);
    ffi_cif *cif = &(af->cif);
    int nargs = lua_gettop(L) - 1;
    if(nargs != af->nparams)
        return luaL_error(L, "alien: too %s arguments (function %s)", nargs < af->nparams ? "few" : "many",
                af->name ? af->name : "anonymous");
    if(nargs > 0) args = alloca(sizeof(void*) * nargs);
    for(i = 0; i < nargs; i++) {
        void *arg;
        int j = i + 2;
        switch(af->params[i]) {
            case AT_byte:
                arg = alloca(sizeof(char)); *((char*)arg) = (signed char)lua_tointeger(L, j);
                break;
            case AT_char:
                arg = alloca(sizeof(unsigned char)); *((unsigned char*)arg) = (unsigned char)lua_tointeger(L, j);
                break;
            case AT_short:
                arg = alloca(sizeof(short)); *((short*)arg) = (short)lua_tonumber(L, j);
                break;
            case AT_ushort:
                arg = alloca(sizeof(unsigned short)); *((unsigned short*)arg) = (unsigned short)lua_tonumber(L, j);
                break;
            case AT_int:
                arg = alloca(sizeof(int)); *((int*)arg) = (int)lua_tonumber(L, j);
                break;
            case AT_uint:
                arg = alloca(sizeof(unsigned int)); *((unsigned int*)arg) = (unsigned int)lua_tonumber(L, j);
                break;
            case AT_long:
                arg = alloca(sizeof(long)); *((long*)arg) = (long)lua_tonumber(L, j);
                break;
            case AT_ulong:
                arg = alloca(sizeof(unsigned long)); *((unsigned long*)arg) = (unsigned long)lua_tonumber(L, j);
                break;
            case AT_ptrdiff_t:
                arg = alloca(sizeof(ptrdiff_t)); *((ptrdiff_t*)arg) = (ptrdiff_t)lua_tonumber(L, j);
                break;
            case AT_size_t:
                arg = alloca(sizeof(size_t)); *((size_t*)arg) = (size_t)lua_tonumber(L, j);
                break;
            case AT_float:
                arg = alloca(sizeof(float)); *((float*)arg) = (float)lua_tonumber(L, j);
                break;
            case AT_double:
                arg = alloca(sizeof(double)); *((double*)arg) = (double)lua_tonumber(L, j);
                break;
            case AT_string:
                arg = alloca(sizeof(char*));
                if(lua_isuserdata(L, j))
                    *((char**)arg) = alien_touserdata(L, j);
                else
                    *((const char**)arg) = lua_isnil(L, j) ? NULL : lua_tostring(L, j);
                break;
            case AT_pointer:
                arg = alloca(sizeof(char*));
                *((void**)arg) = lua_isstring(L, j) ? (void*)lua_tostring(L, j) : alien_touserdata(L, j);
                break;
            case AT_refchar:
                arg = alloca(sizeof(char *));
                *((char **)arg) = alloca(sizeof(char));
                **((char **)arg) = (char)lua_tonumber(L, j);
                nrefc++;
                break;
            case AT_refint:
                arg = alloca(sizeof(int *));
                *((int **)arg) = alloca(sizeof(int));
                **((int **)arg) = (int)lua_tonumber(L, j);
                nrefi++;
                break;
            case AT_refuint:
                arg = alloca(sizeof(unsigned int *));
                *((unsigned int **)arg) = alloca(sizeof(unsigned int));
                **((unsigned int **)arg) = (unsigned int)lua_tonumber(L, j);
                nrefui++;
                break;
            case AT_refdouble:
                arg = alloca(sizeof(double *));
                *((double **)arg) = alloca(sizeof(double));
                **((double **)arg) = (double)lua_tonumber(L, j);
                nrefd++;
                break;
            case AT_longlong:
                arg = alloca(sizeof(long long)); *((long long*)arg) = (long long)lua_tonumber(L, j);
                break;
            case AT_ulonglong:
                arg = alloca(sizeof(unsigned long long)); *((unsigned long long*)arg) = (unsigned long long)lua_tonumber(L, j);
                break;
            case AT_callback:
                arg = alloca(sizeof(void*)); *((void**)arg) = (alien_Function *)alien_checkfunction(L, j)->fn;
                break;
            default:
                return luaL_error(L, "alien: parameter %d is of unknown type (function %s)", j,
                        af->name ? af->name : "anonymous");
        }
        args[i] = arg;
    }
    pret = NULL;
    switch(af->ret_type) {
        case AT_void: ffi_call(cif, af->fn, NULL, args); lua_pushnil(L); break;
        case AT_byte: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (signed char)iret); break;
        case AT_char: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (unsigned char)iret); break;
        case AT_short: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (short)iret); break;
        case AT_ushort: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (unsigned short)iret); break;
        case AT_int: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, iret); break;
        case AT_uint: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (unsigned int)iret); break;
        case AT_long: ffi_call(cif, af->fn, &lret, args); lua_pushnumber(L, lret); break;
        case AT_ulong: ffi_call(cif, af->fn, &ulret, args); lua_pushnumber(L, (unsigned long)ulret); break;
        case AT_ptrdiff_t: ffi_call(cif, af->fn, &lret, args); lua_pushnumber(L, lret); break;
        case AT_size_t: ffi_call(cif, af->fn, &ulret, args); lua_pushnumber(L, (size_t)ulret); break;
        case AT_float: ffi_call(cif, af->fn, &fret, args); lua_pushnumber(L, fret); break;
        case AT_double: ffi_call(cif, af->fn, &dret, args); lua_pushnumber(L, dret); break;
        case AT_string: ffi_call(cif, af->fn, &pret, args);
                        if (pret) lua_pushstring(L, (const char *)pret); else lua_pushnil(L); break;
        case AT_pointer: ffi_call(cif, af->fn, &pret, args);
                         if (pret) lua_pushlightuserdata(L, pret); else lua_pushnil(L); break;
        default:
                         return luaL_error(L, "alien: unknown return type (function %s)", af->name ?
                                 af->name : "anonymous");
    }
    for(i = 0; i < nargs; i++) {
        switch(af->params[i]) {
            case AT_refchar: lua_pushnumber(L, **(char **)args[i]); break;
            case AT_refint: lua_pushnumber(L, **(int **)args[i]); break;
            case AT_refuint: lua_pushnumber(L, **(unsigned int **)args[i]); break;
            case AT_refdouble: lua_pushnumber(L, **(double **)args[i]); break;
            default: break;
        }
    }
    return 1 + nrefi + nrefui + nrefc + nrefd;
}

int alien_function_addr(lua_State *L) {
    alien_Function *af = alien_checkfunction(L, 1);
    lua_pushnumber(L, (size_t)af->fn);
    return 1;
}

int alien_function_gc(lua_State *L) {
    alien_Function *af = alien_checkfunction(L, 1);
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    if(af->name) LALLOC_FREE_STRING(lalloc, aud, af->name);
    if(af->params) lalloc(aud, af->params, sizeof(alien_Type) * af->nparams, 0);
    if(af->ffi_params) lalloc(aud, af->ffi_params, sizeof(ffi_type *) * af->nparams, 0);
    if(af->hookhandle) {
        funchook_uninstall(af->hookhandle, 0);
        funchook_destroy(af->hookhandle);
        af->hookhandle = NULL;
        af->fn = af->scc;
        af->scc = NULL;
    }
    if(af->fn_ref) {
        luaL_unref(af->L, LUA_REGISTRYINDEX, af->fn_ref);
        ffi_closure_free(af->fn);
    }
    return 0;
}

