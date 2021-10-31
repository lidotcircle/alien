#include "alien_library.h"
#include "alien_function.h"
#include "utils.h"
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;


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


alien_Library::alien_Library(lua_State* L, const string& libname, void* libhandle):
    L(L), name(libname), lib(libhandle), funclist()
{
    this->init_func_list = false;
}

const vector<string>& alien_Library::function_list() {
    while (!this->init_func_list) {
        struct function_list* functions = lf_load(this->lib);
        if (functions == nullptr)
            break;

        for (size_t i=0;i<lf_size(functions);i++)
            this->funclist.push_back(lf_index(functions, i));

        lf_free(functions);
        break;
    }
    this->init_func_list = true;

    return this->funclist;
}

bool alien_Library::has_function(const string& funcname) {
    if (!this->init_func_list)
        this->function_list();

    for (auto& f: this->funclist) {
        if (f == funcname)
            return true;
    }

    return false;
}

lua_State* alien_Library::get_lua_State() {
    return this->L;
}

void* alien_Library::loadfunc(const std::string& func) {
    return alien_loadfunc(this->L, this->lib, this->name.c_str());
}

std::string alien_Library::tostring() {
    return "[alien library '" + this->name + "']";
}

const std::string& alien_Library::libname() {
    return this->name;
}

alien_Library::~alien_Library() {
    if (this->lib != nullptr) {
        alien_unload(this->lib);
        this->lib = nullptr;
    }

    this->L = nullptr;
    this->funclist.clear();
    this->init_func_list = false;
}


#define ALIEN_LIBRARY_META "alien_library_meta"
#define ALIEN_LIBRARY_DEFAULT_LIB_GN "__alien_library_default_gn"
#define ALIEN_LIBRARY_MISC_LIB       "__alien_library_misc_lib"

static alien_Library** alien_checklibrary(lua_State*L, int idx) {
    return (alien_Library **)luaL_checkudata(L, idx, ALIEN_LIBRARY_META);
}

static int alien_library_get(lua_State *L);
static int alien_library_tostring(lua_State *L);
static int alien_library_gc(lua_State *L);

int alien_library_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_LIBRARY_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_library_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, alien_library_get);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_library_tostring);
    lua_settable(L, -3);

    lua_pop(L, 1);

    alien_Library* al = new alien_Library(L, "default", nullptr);
    alien_Library** pal = (alien_Library **)lua_newuserdata(L, sizeof(alien_Library*));
    *pal = al;
    luaL_setmetatable(L, ALIEN_LIBRARY_META);

    alien_push_alien(L);
    lua_pushliteral(L, ALIEN_LIBRARY_DEFAULT_LIB_GN);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 2);

    return 0;
}

alien_Library* alien_library__get_default_library(lua_State *L) {
    alien_push_alien(L);
    lua_pushliteral(L, ALIEN_LIBRARY_DEFAULT_LIB_GN);
    lua_gettable(L, -2);
    alien_Library** pal = alien_checklibrary(L, -1);
    return *pal;
}

alien_Library* alien_library__get_misc(lua_State *L) {
    return alien_library__get_default_library(L);
}

int alien_load(lua_State *L) {
    const char *libname = luaL_checkstring(L, lua_gettop(L));
    void* lib = alien_openlib(L, libname);
    if(!lib)
        return lua_error(L);
    alien_Library* al = new alien_Library(L, string(libname), lib);

    alien_Library** pal = (alien_Library **)lua_newuserdata(L, sizeof(alien_Library*));
    if(!pal) return luaL_error(L, "alien: out of memory");
    *pal = al;

    lua_newtable(L);
    lua_setuservalue(L, -2);

    luaL_getmetatable(L, ALIEN_LIBRARY_META);
    lua_setmetatable(L, -2);
    return 1;
}

int alien_library_gc(lua_State *L) {
    alien_Library **pal = alien_checklibrary(L, 1);
    alien_Library *al = *pal;
    *pal = nullptr;
    delete al;
    return 0;
}

int alien_library_get(lua_State *L) {
    alien_Library **pal = alien_checklibrary(L, 1);
    alien_Library *al = *pal;
    const char *funcname = luaL_checkstring(L, 2);

    lua_getuservalue(L, 1);
    int cache = lua_gettop(L);
    lua_getfield(L, cache, funcname);
    if(!lua_isnil(L, -1)) return 1;
    lua_pop(L, 1);

    void* fn = al->loadfunc(funcname);
    int n = alien_function__make_function(L, al, fn, funcname);
    assert( n == 1);

    lua_pushstring(L, funcname);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    return 1;
}

int alien_library_tostring(lua_State *L) {
    alien_Library **pal = alien_checklibrary(L, 1);
    alien_Library *al = *pal;
    lua_pushfstring(L, "alien library %s", al->libname().c_str());
    return 1;
}

int alien_functionlist(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1)
        return luaL_error(L, "alien: too %s arguments[ functionlist(alien_library) ]", 
                          n > 1 ? "many" : "few");

    alien_Library** pal = alien_checklibrary(L, 1);
    alien_Library* al = *pal;

    lua_newtable(L);
    int i = 1;
    for(auto& f: al->function_list()) {
        lua_pushnumber(L, i);
        lua_pushstring(L, f.c_str());
        lua_settable(L, -3);
        i++;
    }

    return 1;
}

int alien_hasfunction(lua_State* L) {
    size_t nargs = lua_gettop(L);
    if (nargs != 2)
        return luaL_error(L, "alien: too %s arguments (function hasfunction)",
                          nargs < 2 ? "few" : "many");

    alien_Library** pal = alien_checklibrary(L, 1);
    alien_Library* al = *pal;
    const char* funcname = luaL_checkstring(L, 2);

    bool ans = al->has_function(funcname);
    lua_pushboolean(L, ans);

    return 1;
}

