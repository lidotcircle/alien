#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef _WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


static char luascriptlrd[1024];
EXPORT int luarun(const char* luamodule) {
    if (luamodule == NULL)
        return 1;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    snprintf(luascriptlrd, sizeof(luascriptlrd), "require(\"%s\")", luamodule);
    if (luaL_loadstring(L, luascriptlrd) != LUA_OK)
        return 1;
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        return 1;
    lua_close(L);
    return 0;
}

EXPORT int luarun_async(const char* luamodule) {
#ifdef _WIN32
    static char fn[MAX_PATH + 1];
    strcpy_s(fn, MAX_PATH, luamodule);
    return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)luarun, fn, 0, NULL) == INVALID_HANDLE_VALUE;
#else
    return 1;
#endif
}
