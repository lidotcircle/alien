#include "alien.h"


void alien_callback_call(ffi_cif *cif, void *resp, void **args, void *data) {
    printf("threadid = 0x%d\n", GetCurrentThreadId());

    alien_Function *ac = (alien_Function *)data;
    printf("callback called.\n");
    printf("nparams = %d, ret_type = %d\n", ac->nparams, ac->ret_type);
    lua_State *L = ac->L;
    int i;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ac->fn_ref);
    for(i = 0; i < ac->nparams; i++) {
        switch(ac->params[i]) {
            case AT_byte: lua_pushnumber(L, (signed char)*((int*)args[i])); break;
            case AT_char: lua_pushnumber(L, (unsigned char)*((int*)args[i])); break;
            case AT_short: lua_pushnumber(L, (short)*((int*)args[i])); break;
            case AT_ushort: lua_pushnumber(L, (unsigned short)*((unsigned int*)args[i])); break;
            case AT_int: lua_pushnumber(L, *((int*)args[i])); break;
            case AT_uint: lua_pushnumber(L, *((unsigned int*)args[i])); break;
            case AT_long: lua_pushnumber(L, (long)*((long*)args[i])); break;
            case AT_ulong: lua_pushnumber(L, (unsigned long)*((unsigned long*)args[i])); break;
            case AT_ptrdiff_t: lua_pushnumber(L, *((ptrdiff_t*)args[i])); break;
            case AT_size_t: lua_pushnumber(L, *((size_t*)args[i])); break;
            case AT_float: lua_pushnumber(L, (float)*((float*)args[i])); break;
            case AT_double: lua_pushnumber(L, *((double*)args[i])); break;
            case AT_string: lua_pushstring(L, *((char**)args[i])); break;
            case AT_pointer:
                            {
                                void *ptr = *((void**)args[i]);
                                ptr ? lua_pushlightuserdata(L, ptr) : lua_pushnil(L);
                            }
                            break;
            case AT_refchar: lua_pushnumber(L, **((unsigned char**)args[i])); break;
            case AT_refint: lua_pushnumber(L, **((int**)args[i])); break;
            case AT_refuint: lua_pushnumber(L, **((unsigned int**)args[i])); break;
            case AT_refdouble: lua_pushnumber(L, **((double**)args[i])); break;
            case AT_longlong:
            case AT_ulonglong:
            default: luaL_error(L, "alien: unknown parameter type in callback");
        }
    }
    printf("callback call lua\n");
    lua_call(L, ac->nparams, 1);
    printf("callback lua return\n");
    switch(ac->ret_type) {
        case AT_void: break;
        case AT_byte: *((int*)resp) = (signed char)lua_tointeger(L, -1); break;
        case AT_char: *((int*)resp) = (unsigned char)lua_tointeger(L, -1); break;
        case AT_short: *((int*)resp) = (short)lua_tonumber(L, -1); break;
        case AT_ushort: *((unsigned int*)resp) = (unsigned short)lua_tonumber(L, -1); break;
        case AT_int: *((int*)resp) = (int)lua_tonumber(L, -1); break;
        case AT_uint: *((unsigned int*)resp) = (unsigned int)lua_tonumber(L, -1); break;
        case AT_long: *((long*)resp) = (long)lua_tonumber(L, -1); break;
        case AT_ulong: *((unsigned long*)resp) = (unsigned long)lua_tonumber(L, -1); break;
        case AT_ptrdiff_t: *((ptrdiff_t*)resp) = (ptrdiff_t)lua_tonumber(L, -1); break;
        case AT_size_t: *((size_t*)resp) = (size_t)lua_tonumber(L, -1); break;
        case AT_float: *((float*)resp) = (float)lua_tonumber(L, -1); break;
        case AT_double: *((double*)resp) = (double)lua_tonumber(L, -1); break;
        case AT_string: *((char**)resp) = lua_isuserdata(L, -1) ? alien_touserdata(L, -1) : (char *)lua_tostring(L, -1); break;
        case AT_pointer: *((void**)resp) = lua_isstring(L, -1) ? (void*)lua_tostring(L, -1) : alien_touserdata(L, -1); break;
        case AT_longlong:
                         if(lua_isnumber(ac->L, -1))
                             *((long long*)resp) = (long long)lua_tonumber(ac->L, -1);
                         else
                             *((void**)resp) = lua_touserdata(ac->L, -1);
                         break;
        case AT_ulonglong:
                         if(lua_isnumber(ac->L, -1))
                             *((unsigned long long*)resp) = (unsigned long long)lua_tonumber(ac->L, -1);
                         else
                             *((void**)resp) = lua_touserdata(ac->L, -1);
                         break;
        default: luaL_error(L, "alien: unknown return type in callback");
    }
    lua_pop(ac->L, 1);
    printf("callback return\n");
}

int alien_callback_new(lua_State *L) {
    alien_Function *ac;
    ffi_status status;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    ac = (alien_Function *)lua_newuserdata(L, sizeof(alien_Function));
    if(!ac) return luaL_error(L, "alien: out of memory");
    ac->fn = ffi_closure_alloc(sizeof(ffi_closure), &ac->ffi_codeloc);
    if(ac->fn == NULL) return luaL_error(L, "alien: cannot allocate callback");
    ac->L = L;
    ac->ret_type = AT_void;
    ac->ffi_ret_type = &ffi_type_void;
    ac->nparams = 0;
    ac->params = NULL;
    ac->ffi_params = NULL;
    ac->type_ref = -1;
    lua_pushvalue(L, 1);
    ac->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, ALIEN_FUNCTION_META);
    lua_setmetatable(L, -2);
    status = ffi_prep_cif(&(ac->cif), FFI_DEFAULT_ABI, ac->nparams,
            ac->ffi_ret_type, ac->ffi_params);
    if(status == FFI_OK)
        status = ffi_prep_closure_loc(ac->fn, &(ac->cif), &alien_callback_call, ac, ac->ffi_codeloc);
    if(status != FFI_OK) {
        ffi_closure_free(ac->fn);
        return luaL_error(L, "alien: cannot create callback");
    }
    ac->lib = NULL;
    ac->name = NULL;
    ac->hookhandle = NULL;
    ac->trampoline_fn = NULL;
    return 1;
}

