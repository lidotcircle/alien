#include <funchook.h>
#include <lua.hpp>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "alien.h"
#include "alien_exception.h"
#include "alien_function.h"
#include "alien_value_callback.h"
#include "alien_value.h"
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

ffi_abi alien_checkabi(lua_State* L, int idx) {
    if (!lua_isstring(L, idx))
        throw AlienInvalidArgumentException("abi should be specified as string");

    string na(lua_tostring(L, idx));
    bool found = false;
    for (size_t i=0;ffi_abi_names[i];i++) {
        if (na == string(ffi_abi_names[i])) {
            found = true;
            break;
        }
    }
    if (!found)
        throw AlienNotImplementedException("unsupported abi '%s' in current platform", na.c_str());

    ffi_abi abi = ffi_abis[luaL_checkoption(L, idx, "default", ffi_abi_names)];
    return abi;
}

#define ALIEN_FUNCTION_META "alien_function_meta"

static alien_Function *alien_checkfunction(lua_State *L, int index) {
    return *static_cast<alien_Function **>(luaL_checkudata(L, index, ALIEN_FUNCTION_META));
}
static bool alien_isfunction(lua_State *L, int index) {
    return luaL_testudata(L, index, ALIEN_FUNCTION_META) != nullptr;
}

ALIEN_LUA_FUNC static int alien_function_gc(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_tostring(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_call(lua_State *L);
ALIEN_LUA_FUNC static int alien_function__index(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_types(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_hook(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_unhook(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_addr(lua_State *L);
ALIEN_LUA_FUNC static int alien_function_trampoline(lua_State *L);

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

    lua_pop(L, 1);
    return 0;
}

static map<string, lua_CFunction> function_methods = {
    { "types",  alien_function_types },
    { "hook",   alien_function_hook },
    { "unhook", alien_function_unhook },
};
static map<string, lua_CFunction> function_properties = {
    { "addr",       alien_function_addr },
    { "trampoline", alien_function_trampoline },
};
ALIEN_LUA_FUNC static int alien_function__index(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
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
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            const char* erro = lua_tostring(L, -1);
            throw AlienLuaThrow("%s", erro);
        }
        return 1;
    }

    throw AlienNotImplementedException("method / properties %s not found in alien_function", method.c_str());
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_call(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function* af = alien_checkfunction(L, 1);
    return af->call_from_lua(L);
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_tostring(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function* af = alien_checkfunction(L, 1);
    lua_pushstring(L, af->tostring().c_str());
    return 1;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_gc(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function* af = alien_checkfunction(L, 1);
    delete af;
    return 1;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_types(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function *af = alien_checkfunction(L, 1);

    ffi_abi abi = FFI_DEFAULT_ABI;
    alien_type* ret = nullptr;
    vector<alien_type*> params;
    bool is_variadic = false;
    std::tie(abi, ret, params, is_variadic) = alien_function__parse_types_table(L, 2);

    if (!af->define_types(abi, ret, params, is_variadic))
        throw AlienException("define function type failed");

    return 0;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_hook(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function *af = alien_checkfunction(L, 1);
    alien_type* cbtype = alien_type_byname(L, "callback");

    if (lua_isinteger(L, 2)) {
        void* fn = reinterpret_cast<void*>(lua_tointeger(L, 2));
        af->hook(L, fn, LUA_NOREF);
    } else if (lua_islightuserdata(L, 2)) {
        void* fn = lua_touserdata(L, 2);
        af->hook(L, fn, LUA_NOREF);
    } else if (alien_isfunction(L, 2)) {
        alien_Function *afx = alien_checkfunction(L, 2);
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        af->hook(L, afx->funcaddr(), ref);
    } else if (alien_value_callback::is_this_value(cbtype, L, 2)) {
        alien_value_callback *cb = alien_value_callback::checkvalue(cbtype, L, 2);
        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        af->hook(L, cb->funcaddr(), ref);
    } else {
        throw AlienInvalidArgumentException("unexpected hook parameter");
    }

    return 1;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_unhook(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function *af = alien_checkfunction(L, 1);
    af->unhook(L);
    return 1;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_addr(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function *af = alien_checkfunction(L, 1);
    lua_pushnumber(L, reinterpret_cast<ptrdiff_t>(af->funcaddr()));
    return 1;
    ALIEN_EXCEPTION_END();
}

ALIEN_LUA_FUNC static int alien_function_trampoline(lua_State* L) {
    ALIEN_EXCEPTION_BEGIN();
    alien_Function *af = alien_checkfunction(L, 1);
    return af->trampoline(L);
    ALIEN_EXCEPTION_END();
}

int alien_function__make_function(lua_State *L, alien_Library* lib, void *fn, const string& name) {
    assert(L == lib->get_lua_State());
    alien_Function* af = new alien_Function(lib, fn, name);
    alien_Function** paf = (alien_Function**)lua_newuserdata(L, sizeof(alien_Function*));
    *paf = af;
    luaL_setmetatable(L, ALIEN_FUNCTION_META);
    return 1;
}

std::tuple<ffi_abi,alien_type*,std::vector<alien_type*>,bool>
    alien_function__parse_types_table(lua_State *L, int idx) 
{
    vector<alien_type*> params;

    if (!lua_istable(L, idx))
        throw AlienInvalidArgumentException("bad type definition");

    bool is_variadic = false;
    ffi_abi abi = FFI_DEFAULT_ABI;
    alien_type* ret = nullptr;

    lua_getfield(L, idx, "is_variadic");
    if (lua_isboolean(L, -1))
        is_variadic = lua_toboolean(L, -1);

    lua_getfield(L, idx, "abi");
    if (!lua_isnil(L, -1))
        abi = alien_checkabi(L, -1);

    lua_getfield(L, idx, "ret");
    if (lua_isnil(L, -1))
        throw AlienInvalidArgumentException("return type should be specified");
    ret = alien_checktype(L, -1);
    lua_pop(L, 3);

    size_t nparams = luaL_len(L, idx);
    for(size_t i=1;i<=nparams;i++) {
        lua_rawgeti(L, idx, i);

        alien_type* para_type = alien_checktype(L, -1);
        params.push_back(para_type);

        lua_pop(L, 1);
    }

    return std::make_tuple(abi, ret, params, is_variadic);
}

ALIEN_LUA_FUNC int alien_function_new(lua_State *L) {
    ALIEN_EXCEPTION_BEGIN();
    void* fn = nullptr;
    if (!lua_isinteger(L, 1)) {
        fn = reinterpret_cast<void*>(lua_tointeger(L, 2));
    } else if (lua_islightuserdata(L, 2)) {
        fn = lua_touserdata(L, 2);
    } else {
        throw AlienInvalidArgumentException("require a function pointer (integer or lightuserdata)");
    }

    auto lib = alien_library__get_misc(L);
    std::ostringstream oss;
    oss << "funcptr#[0x" << std::hex << fn << "]";
    alien_function__make_function(L, lib, fn, oss.rdbuf()->str());

    return 1;
    ALIEN_EXCEPTION_END();
}

alien_Function::alien_Function(alien_Library* lib, void* fn, const string& name):
    lib(lib), name(name), L(lib->get_lua_State()), fn(fn),
    ret_type(nullptr), params(), ffi_params(nullptr),
    hookhandle(nullptr), trampoline_ref(LUA_NOREF), keepalive_ref(LUA_NOREF),
    is_variadic(false)
{
    alien_type* void_t = alien_type_byname(L, "void");
    this->define_types(FFI_DEFAULT_ABI, void_t, vector<alien_type*>(), false);
}

bool alien_Function::define_types(ffi_abi abi, alien_type* ret, const vector<alien_type*>& params, bool is_variadic) {
    this->is_variadic = is_variadic;
    this->params = params;
    this->ret_type = ret;

    this->ffi_params = std::make_unique<ffi_type*[]>(this->params.size());
    for(size_t i=0;i<this->params.size();i++)
        this->ffi_params[i] = this->params[i]->ffitype();

    ffi_status status = ffi_prep_cif(&this->cif, abi, this->params.size(),
                                     this->ret_type->ffitype(), this->ffi_params.get());
    if(status != FFI_OK)
        return false;

    return true;
}

int alien_Function::call_from_lua(lua_State *L) {
    assert(this->lib->get_lua_State() == L);

    unique_ptr<void*[]> args;
    int nargs = lua_gettop(L) - 1;
    if (nargs < this->params.size() || (!this->is_variadic && nargs > this->params.size()))
        throw AlienInvalidArgumentException(
                "alien: too %s arguments (function %s), expect %ld but get %d",
                nargs < this->params.size() ? "few" : "many",
                this->name.c_str(), this->params.size(), nargs);

    if (nargs > 0) args = make_unique<void*[]>(nargs);
    vector<std::unique_ptr<alien_value>> values;
    int i = 0;
    for (; i < this->params.size(); i++) {
        alien_value* val = this->params[i]->from_lua(L, i + 2);
        args[i] = val->ptr();
        values.push_back(std::unique_ptr<alien_value>(val));
    }
    for (; i < nargs; i++) {
        alien_value* val = alien_generic_from_lua(L, i + 2);
        args[i] = val->ptr();
        values.push_back(std::unique_ptr<alien_value>(val));
    }
    std::unique_ptr<alien_value> ret(this->ret_type->new_value(L));

    ffi_cif var_cif;
    std::unique_ptr<ffi_type*[]> ffi_params;
    ffi_cif* cif = &this->cif;
    if (this->is_variadic) {
        ffi_params = std::make_unique<ffi_type*[]>(nargs);
        int k = 0;
        for (;k<this->params.size();k++)
            ffi_params[k] = this->ffi_params[k];
        for (;k<nargs;k++)
            ffi_params[k] = const_cast<ffi_type*>(values[k]->alientype()->ffitype());

        ffi_prep_cif_var(&var_cif, this->cif.abi, this->cif.nargs, nargs,
                         this->cif.rtype, ffi_params.get());
        cif = &var_cif;
    }

    ffi_call(cif, reinterpret_cast<void(*)()>(this->fn), ret->ptr(), args.get());
    ret->to_lua(L);

    int nref = 0;
    for (size_t i=0;i<params.size();i++) {
        auto vt = this->params[i];

        if (vt->is_ref() && vt->is_basic()) {
            values[i]->to_lua(L);
            nref++;
        }
    }

    return 1 + nref;
}

int alien_Function::hook(lua_State* L, void* jmpto, int objref) {
    if (this->hookhandle != nullptr)
        throw AlienException("function has been hooked");

    funchook_t* hook = funchook_create();
    void* fn = this->fn;
    if (funchook_prepare(hook, &fn, jmpto) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        throw AlienException("prepare hook failed");
    }
    if (funchook_install(hook, 0) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        throw AlienException("install hook failed");
    }

    int n = alien_function__make_function(L, this->lib, fn, "trampoline#" + this->name);
    assert(n == 1);
    alien_Function* trampoline_func = alien_checkfunction(L, -1);
    trampoline_func->define_types(this->cif.abi, this->ret_type, this->params, this->is_variadic);

    lua_pushvalue(L, -1);
    this->hookhandle = hook;
    this->trampoline_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    this->keepalive_ref = objref;

    return 1;
}

int alien_Function::unhook(lua_State* L) {
    assert(L == this->L);
    if (this->hookhandle == nullptr) {
        throw AlienException("not hooked");
    }

    if (funchook_uninstall(static_cast<funchook_t*>(this->hookhandle), 0) != FUNCHOOK_ERROR_SUCCESS) {
        throw AlienException("uninstall hook failed");
    }

    funchook_destroy(static_cast<funchook_t*>(this->hookhandle));
    this->hookhandle = nullptr;
    luaL_unref(this->L, LUA_REGISTRYINDEX, this->trampoline_ref);
    this->trampoline_ref = LUA_NOREF;

    return 0;
}

int  alien_Function::trampoline(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, this->trampoline_ref);
    return 1;
}

string alien_Function::tostring() {
    return this->lib->libname() + "::" + this->name + "()";
}

void* alien_Function::funcaddr() {
    return this->fn;
}

alien_Function::~alien_Function() {
    if (this->hookhandle)
        this->unhook(this->L);

    luaL_unref(this->L, LUA_REGISTRYINDEX, this->keepalive_ref);
    this->keepalive_ref = LUA_NOREF;
}

