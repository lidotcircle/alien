#include <funchook.h>
#include <lua.hpp>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "alien_function.h"
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

int alien_function__make_function(lua_State *L, alien_Library* lib, void *fn, const string& name) {
    assert(L == lib->get_lua_State());
    alien_Function* af = new alien_Function(lib, fn, name);
    alien_Function** paf = (alien_Function**)lua_newuserdata(L, sizeof(alien_Function*));
    *paf = af;
    luaL_setmetatable(L, ALIEN_FUNCTION_META);
    return 1;
}

// TODO
int alien_function_new(lua_State *L) {
    return 0;
}


alien_Function::alien_Function(alien_Library* lib, void* fn, const string& name):
    lib(lib), name(name), L(lib->get_lua_State()), fn(fn) , ret_type(nullptr),
    params(), ffi_params(nullptr), hookhandle(nullptr)
{
    // TODO
    alien_type* void_t = nullptr;
    this->define_types(FFI_DEFAULT_ABI, void_t, vector<alien_type*>());
}

bool alien_Function::define_types(ffi_abi abi, alien_type* ret, const vector<alien_type*>& params) {
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
    int iret; double dret; void *pret; long lret; unsigned long ulret; float fret;
    int i, nrefi = 0, nrefui = 0, nrefd = 0, nrefc = 0;

    void **args = NULL;
    int nargs = lua_gettop(L) - 1;
    if (nargs != this->params.size())
        return luaL_error(L, "alien: too %s arguments (function %s)",
                          nargs < this->params.size() ? "few" : "many",
                          this->name.c_str());
    if(nargs > 0) args = new void*[nargs];
    vector<std::unique_ptr<alien_value>> values;
    for(i = 0; i < nargs; i++) {
        alien_value* val = this->params[i]->fromLua(L, i + 2);
        args[i] = val->ptr();
        values.push_back(std::unique_ptr<alien_value>(val));
    }
    std::unique_ptr<alien_value> ret(this->ret_type->new_value());
    ffi_call(&this->cif, reinterpret_cast<void(*)()>(this->fn), ret->ptr(), args);
    ret->toLua(L);

    int nref = 0;
    for (size_t i=0;i<values.size();i++) {
        auto vt = this->params[i];

        if (vt->is_ref() && vt->is_basic()) {
            values[i]->toLua(L);
            nref++;
        }
    }

    return 1 + nref;
}

int alien_Function::hook(lua_State* L, void* jmpto, int objref) {
    if (this->hookhandle != nullptr)
        return luaL_error(L, "alien: function has been hooked");

    funchook_t* hook = funchook_create();
    void* fn = this->fn;
    if (funchook_prepare(hook, &fn, jmpto) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: prepare hook failed");
    }
    if (funchook_install(hook, 0) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(hook);
        return luaL_error(L, "alien: install hook failed");
    }

    int n = alien_function__make_function(L, this->lib, fn, "trampoline#" + this->name);
    assert(n == 1);
    lua_pushvalue(L, -1);
    this->trampoline_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    this->keepalive_ref = objref;

    return 1;
}

int alien_Function::unhook(lua_State* L) {
    assert(L == this->L);
    if (this->hookhandle == nullptr) {
        return luaL_error(L, "alien: not hooked");
    }

    if (funchook_uninstall(static_cast<funchook_t*>(this->hookhandle), 0) != FUNCHOOK_ERROR_SUCCESS) {
        return luaL_error(L, "alien: uninstall hook failed");
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

