#include <funchook.h>
#include <lua.hpp>
#include <string>
#include <vector>
#include <map>
#include "alien_function.h"
using namespace std;


#if defined(X86_WIN32)
const ffi_abi ffi_abis[] = { FFI_SYSV, FFI_STDCALL, FFI_THISCALL, FFI_FASTCALL, FFI_MS_CDECL, FFI_PASCAL, FFI_REGISTER, FFI_DEFAULT_ABI };
const char *const ffi_abi_names[] = { "sysv", "stdcall", "thiscall", "fastcall", "cdecl", "pascal", "register", "default", NULL };
#elif defined(X86_WIN64)
const ffi_abi ffi_abis[] = { FFI_WIN64, FFI_GNUW64, FFI_DEFAULT_ABI };
const char *const ffi_abi_names[] = { "win64", "gnuw64", "default", NULL };
#elif defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
const ffi_abi ffi_abis[] = { FFI_UNIX64, FFI_WIN64, FFI_EFI64, FFI_GNUW64, FFI_DEFAULT_ABI };
const char *const ffi_abi_names[] = { "unix64", "win64", "efi64", "gnuw64", "default", NULL };
#else
const ffi_abi ffi_abis[] = { FFI_SYSV, FFI_THISCALL, FFI_FASTCALL, FFI_STDCALL, FFI_PASCAL, FFI_REGISTER, FFI_MS_CDECL, FFI_DEFAULT_ABI };
const char *const ffi_abi_names[] = { "sysv", "thiscall", "fastcall", "stdcall", "pascal", "register", "cdecl", "default", NULL };
#endif

#define ALIEN_FUNCTION_META "alien_function_meta"

static alien_Function **alien_checkfunction(lua_State *L, int index) {
    return (alien_Function **)luaL_checkudata(L, index, ALIEN_FUNCTION_META);
}
static bool alien_isfunction(lua_State *L, int index) {
    return luaL_testudata(L, index, ALIEN_FUNCTION_META) != nullptr;
}

static int alien_function_gc(lua_State *L);
static int alien_function_tostring(lua_State *L);
static int alien_function_call(lua_State *L);
static int alien_function__index(lua_State *L);
static int alien_function_types(lua_State *L);
static int alien_function_hook(lua_State *L);
static int alien_function_unhook(lua_State *L);
static int alien_function_addr(lua_State *L);
static int alien_function_trampoline(lua_State *L);

int alien_function_init(lua_State *L) {
    luaL_newmetatable(L, ALIEN_FUNCTION_META);
    
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_function_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_function_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__call");
    lua_pushcfunction(L, alien_function_call);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_function__index);
    lua_settable(L, -3);

    return 0;
}

static map<string, lua_CFunction> function_methods = {
    { "types",  alien_function_types },
    { "hook",   alien_function_hook },
    { "unhook", alien_function_unhook },
};
static map<string, lua_CFunction> function_properties = {
    { "addr",       alien_function_types },
    { "trampoline", alien_function_types },
};
static int alien_function__index(lua_State* L) {
    const string method = luaL_checkstring(L, 2);
    auto fn = function_methods.find(method);
    if (fn != function_methods.end()) {
        lua_pushcfunction(L, fn->second);
        return 1;
    }

    fn = function_properties.find(method);
    if (fn != function_properties.end()) {
        lua_pushcfunction(L, fn->second);
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
        return 1;
    }

    return luaL_error(L, "alien: not found method / properties %s in alien_function", method.c_str());
}

static int alien_function_call(lua_State* L) {
    alien_Function** paf = alien_checkfunction(L, 1);
    alien_Function* af = *paf;

    return af->call_from_lua(L);
}

static int alien_function_tostring(lua_State* L) {
    alien_Function** paf = alien_checkfunction(L, 1);
    alien_Function* af = *paf;

    lua_pushstring(L, af->tostring().c_str());
    return 1;
}

static int alien_function_gc(lua_State* L) {
    alien_Function** paf = alien_checkfunction(L, 1);
    alien_Function* af = *paf;
    *paf = nullptr;
    delete af;
    return 1;
}

static int alien_function_types(lua_State* L) {
    alien_Function **paf = alien_checkfunction(L, 1);
    alien_Function *af = *paf;

    alien_type* ret = nullptr;
    vector<alien_type*> params;

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "alien: bad type definition");
    }

    lua_getfield(L, 2, "abi");
    ffi_abi abi = ffi_abis[luaL_checkoption(L, -1, "default", ffi_abi_names)];
    lua_getfield(L, 2, "ret");
    if (lua_isnil(L, -1))
        return luaL_error(L, "alien: return type should be specified");
    ret = alien_checktype(L, -1);
    lua_pop(L, 2);

    size_t nparams = luaL_len(L, 2);
    for(size_t i=1;i<=nparams;i++) {
        lua_rawgeti(L, 2, i);

        alien_type* para_type = alien_checktype(L, -1);
        params.push_back(para_type);

        lua_pop(L, 1);
    }

    if (!af->define_types(abi, ret, params)) {
        return luaL_error(L, "alien: define function type failed");
    }
    return 0;
}

static int alien_function_hook(lua_State* L) {
    alien_Function **paf = alien_checkfunction(L, 1);
    alien_Function *af = *paf;

    if (!lua_isinteger(L, 2)) {
        void* fn = reinterpret_cast<void*>(lua_tointeger(L, 2));
        af->hook(L, fn, LUA_NOREF);
    } else if (lua_islightuserdata(L, 2)) {
        void* fn = lua_touserdata(L, 2);
        af->hook(L, fn, LUA_NOREF);
    } else if (alien_isfunction(L, 2)) {
        alien_Function **pafx = alien_checkfunction(L, 2);
        alien_Function *afx = *pafx;
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        af->hook(L, afx->funcaddr(), ref);
    /* TODO
    } else if (alien_iscallback(L, 2)) {
        alien_type_callback **pcb = alien_checkcallback(L, 2);
        alien_type_callback *cb = *pcb;
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        af->hook(L, cb->funcaddr(), ref);
    */
    } else {
        return luaL_error(L, "alien: unexpected hook parameter");
    }

    return 1;
}

static int alien_function_unhook(lua_State* L) {
    alien_Function **paf = alien_checkfunction(L, 1);
    alien_Function *af = *paf;

    af->unhook(L);
    return 1;
}

static int alien_function_addr(lua_State* L) {
    alien_Function **paf = alien_checkfunction(L, 1);
    alien_Function *af = *paf;

    lua_pushnumber(L, reinterpret_cast<ptrdiff_t>(af->funcaddr()));
    return 1;
}

static int alien_function_trampline(lua_State* L) {
    alien_Function **paf = alien_checkfunction(L, 1);
    alien_Function *af = *paf;

    return af->trampoline(L);
}

// TODO
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
    if (af->type_ref != -1) {
        return luaL_error(L, "alien: function prototype has be defined");
    }
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "alien: define type failed");
    }

    if (af->hookhandle) {
        return luaL_error(L, "aline: type of horigin function must be confirmed when creating");
    }
    int i, ret_type;
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    lua_getfield(L, 2, "ret");
    ret_type = luaL_checkoption(L, -1, "int", alien_typenames);
    af->ret_type = static_cast<alien_Type>(ret_type);
    af->ffi_ret_type = ffitypes[ret_type];
    lua_getfield(L, 2, "abi");
    abi = ffi_abis[luaL_checkoption(L, -1, "default", ffi_abi_names)];
    lua_pop(L, 2);

    if(af->params) {
        lalloc(aud, af->params, sizeof(alien_Type) * af->nparams, 0);
        lalloc(aud, af->ffi_params, sizeof(ffi_type *) * af->nparams, 0);
        af->params = NULL; af->ffi_params = NULL;
    }
    af->nparams = lua_objlen(L, 2);
    if(af->nparams > 0) {
        af->ffi_params = (ffi_type **)lalloc(aud, NULL, 0, sizeof(ffi_type *) * af->nparams);
        if(!af->ffi_params) return luaL_error(L, "alien: out of memory");
        af->params = (alien_Type *)lalloc(aud, NULL, 0, af->nparams * sizeof(alien_Type));
        if(!af->params) return luaL_error(L, "alien: out of memory");
    } else {
        af->ffi_params = NULL;
        af->params = NULL;
    }
    for(i = 0; i < af->nparams; i++) {
        int type;
        lua_rawgeti(L, 2, i + 1);
        type = luaL_checkoption(L, -1, "int", alien_typenames);
        af->params[i] = static_cast<alien_Type>(type);
        af->ffi_params[i] = ffitypes[type];
        lua_pop(L, 1);
    }

    status = ffi_prep_cif(&(af->cif), abi, af->nparams,
            af->ffi_ret_type,
            af->ffi_params);
    if(status != FFI_OK)
        return luaL_error(L, "alien: error in libffi preparation");
    if(af->fn_ref) {
        status = ffi_prep_closure_loc(static_cast<ffi_closure*>(af->fn), &(af->cif), &alien_callback_call, af, af->ffi_codeloc);
        if(status != FFI_OK) return luaL_error(L, "alien: cannot create callback");
    }

    lua_pushvalue(L, 2);
    af->type_ref = luaL_ref(L, LUA_REGISTRYINDEX);
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
    if(nargs > 0) args = static_cast<void**>(alloca(sizeof(void*) * nargs));
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
                    *((char**)arg) = static_cast<char*>(alien_touserdata(L, j));
                else
                    *((const char**)arg) = lua_isnil(L, j) ? NULL : lua_tostring(L, j);
                break;
            case AT_pointer:
                arg = alloca(sizeof(char*));
                *((void**)arg) = lua_isstring(L, j) ? (void*)lua_tostring(L, j) : alien_touserdata(L, j);
                break;
            case AT_refchar:
                arg = alloca(sizeof(char *));
                *((char **)arg) = static_cast<char*>(alloca(sizeof(char)));
                **((char **)arg) = (char)lua_tonumber(L, j);
                nrefc++;
                break;
            case AT_refint:
                arg = alloca(sizeof(int *));
                *((int **)arg) = static_cast<int*>(alloca(sizeof(int)));
                **((int **)arg) = (int)lua_tonumber(L, j);
                nrefi++;
                break;
            case AT_refuint:
                arg = alloca(sizeof(unsigned int *));
                *((unsigned int **)arg) = static_cast<unsigned int*>(alloca(sizeof(unsigned int)));
                **((unsigned int **)arg) = (unsigned int)lua_tonumber(L, j);
                nrefui++;
                break;
            case AT_refdouble:
                arg = alloca(sizeof(double *));
                *((double **)arg) = static_cast<double*>(alloca(sizeof(double)));
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
        case AT_void: ffi_call(cif, (void(*)())(af->fn), NULL, args); lua_pushnil(L); break;
        case AT_byte: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, (signed char)iret); break;
        case AT_char: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, (unsigned char)iret); break;
        case AT_short: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, (short)iret); break;
        case AT_ushort: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, (unsigned short)iret); break;
        case AT_int: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, iret); break;
        case AT_uint: ffi_call(cif, (void(*)())(af->fn), &iret, args); lua_pushnumber(L, (unsigned int)iret); break;
        case AT_long: ffi_call(cif, (void(*)())(af->fn), &lret, args); lua_pushnumber(L, lret); break;
        case AT_ulong: ffi_call(cif, (void(*)())(af->fn), &ulret, args); lua_pushnumber(L, (unsigned long)ulret); break;
        case AT_ptrdiff_t: ffi_call(cif, (void(*)())(af->fn), &lret, args); lua_pushnumber(L, lret); break;
        case AT_size_t: ffi_call(cif, (void(*)())(af->fn), &ulret, args); lua_pushnumber(L, (size_t)ulret); break;
        case AT_float: ffi_call(cif, (void(*)())(af->fn), &fret, args); lua_pushnumber(L, fret); break;
        case AT_double: ffi_call(cif, (void(*)())(af->fn), &dret, args); lua_pushnumber(L, dret); break;
        case AT_string: ffi_call(cif, (void(*)())(af->fn), &pret, args);
                        if (pret) lua_pushstring(L, (const char *)pret); else lua_pushnil(L); break;
        case AT_pointer: ffi_call(cif, (void(*)())(af->fn), &pret, args);
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
    printf("function gc %s\n", af->name);
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    if(af->name) LALLOC_FREE_STRING(lalloc, aud, af->name);
    if(af->params) lalloc(aud, af->params, sizeof(alien_Type) * af->nparams, 0);
    if(af->ffi_params) lalloc(aud, af->ffi_params, sizeof(ffi_type *) * af->nparams, 0);
    if(af->hookhandle) {
        funchook_uninstall(static_cast<funchook_t*>(af->hookhandle), 0);
        funchook_destroy(static_cast<funchook_t*>(af->hookhandle));
        af->hookhandle = NULL;
    }
    /*
    if(af->fn_ref) {
        luaL_unref(af->L, LUA_REGISTRYINDEX, af->fn_ref);
        ffi_closure_free(af->fn);
    }
    */
    return 0;
}

