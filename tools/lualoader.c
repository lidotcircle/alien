#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


static lua_State* GL = NULL;
EXPORT int luarun(const char* luastring) {
    if (GL == NULL) {
        GL = luaL_newstate();
        luaL_openlibs(GL);
    }

    if (luastring == NULL)
        return 1;

    if (luaL_loadstring(GL, luastring) != LUA_OK)
        return 1;
    if (lua_pcall(GL, 0, LUA_MULTRET, 0) != LUA_OK)
        return 1;

    return 0;
}

EXPORT int luaclean(const char* u) {
    if (GL != NULL) {
        lua_close(GL);
        GL = NULL;
    }

    return 0;
}

EXPORT int luarun_once(const char* luastring) {
    if (luastring == NULL)
        return 1;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_loadstring(L, luastring) != LUA_OK) {
        lua_close(L);
        return 1;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
        lua_close(L);
        return 1;
    }
    lua_close(L);
    return 0;
}

EXPORT int luarun_once_async(const char* luastring) {
#ifdef _WIN32
    char* fn = (char*)malloc(strlen(luastring) + 1);
    strcpy(fn, luastring);
    int status = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)luarun_once, fn, 0, NULL);
    free(fn);
    return status == INVALID_HANDLE_VALUE;
#else
    return 1;
#endif
}

EXPORT int luarun_async(const char* luastring) {
#ifdef _WIN32
    char* fn = (char*)malloc(strlen(luastring) + 1);
    strcpy(fn, luastring);
    int status = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)luarun, fn, 0, NULL);
    free(fn);
    return status == INVALID_HANDLE_VALUE;
#else
    return 1;
#endif
}
