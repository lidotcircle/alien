#include "alien.h"


static char alien_buffer_empty; /* Zero-length buffer */

int alien_buffer_new(lua_State *L) {
    size_t size = 0;
    void *p;
    alien_Buffer *ud;
    if(lua_type(L, 1) == LUA_TLIGHTUSERDATA) {
        p = lua_touserdata(L, 1);
    } else {
        const char *s; char *b;
        void *aud;
        lua_Alloc lalloc = lua_getallocf(L, &aud);
        if(lua_type(L, 1) == LUA_TSTRING) {
            s = lua_tolstring(L, 1, &size);
            size++;
        } else {
            s = NULL;
            size = luaL_optinteger(L, 1, BUFSIZ);
        }
        b = size == 0 ? &alien_buffer_empty : (char *)lalloc(aud, NULL, 0, size);
        if(b == NULL)
            return luaL_error(L, "alien: cannot allocate buffer");
        if(s) {
            memcpy(b, s, size - 1);
            b[size - 1] = '\0';
        }
        p = b;
    }
    ud = static_cast<alien_Buffer*>(lua_newuserdata(L, sizeof(alien_Buffer)));
    if (!ud)
        return luaL_error(L, "alien: cannot allocate buffer");
    ud->p = static_cast<char*>(p);
    ud->size = size;
    luaL_getmetatable(L, ALIEN_BUFFER_META);
    lua_setmetatable(L, -2);
    return 1;
}

int alien_buffer_gc(lua_State *L) {
    alien_Buffer *ab = alien_checkbuffer(L, 1);
    if (ab->size > 0) {
        void *aud;
        lua_Alloc lalloc = lua_getallocf(L, &aud);
        lalloc(aud, ab->p, ab->size, 0);
    }
    lua_pop(L, 2);
    return 0;
}

int alien_buffer_length(lua_State *L) {
    lua_pushinteger(L, alien_checkbuffer(L, 1)->size);
    return 1;
}

int alien_buffer_tostring(lua_State *L) {
    size_t size;
    ptrdiff_t offset;
    alien_Buffer *ab = alien_checkbuffer(L, 1);
    if(lua_gettop(L) < 2 || lua_isnil(L, 2)) {
        size = strlen(ab->p);
        offset = 0;
    } else {
        size = luaL_checkinteger(L, 2);
        offset = luaL_optinteger(L, 3, 1) - 1;
    }
    lua_pushlstring(L, ab->p + offset, size);
    return 1;
}

int alien_buffer_strlen(lua_State *L) {
    char *b = alien_checkbuffer(L, 1)->p;
    lua_pushinteger(L, strlen(b));
    return 1;
}

int alien_buffer_topointer(lua_State *L) {
    char *b = alien_checkbuffer(L, 1)->p;
    ptrdiff_t offset = luaL_optinteger(L, 2, 1) - 1;
    lua_pushlightuserdata(L, b + offset);
    return 1;
}

int alien_buffer_tooffset(lua_State *L) {
    char *b = alien_checkbuffer(L, 1)->p;
    char *p;
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA);
    p = (char*)lua_touserdata(L, 2);
    /* It would be nice to do a bounds check, but comparing pointers
       that don't point to the same object has undefined behavior. */
    lua_pushinteger(L, p - b + 1);
    return 1;
}

int alien_buffer_set(lua_State *L) {
    char *b = alien_checkbuffer(L, 1)->p;
    ptrdiff_t offset = luaL_checkinteger(L, 2) - 1;
    int type = luaL_checkoption(L, 4, "char", alien_typenames);
    switch(type) {
        case AT_byte: b[offset] = (signed char)lua_tointeger(L, 3); break;
        case AT_char: b[offset] = (unsigned char)lua_tointeger(L, 3); break;
        case AT_short: *((short*)(&b[offset])) = (short)lua_tonumber(L, 3); break;
        case AT_ushort: *((unsigned short*)(&b[offset])) = (unsigned short)lua_tonumber(L, 3); break;
        case AT_int: *((int*)(&b[offset])) = (int)lua_tonumber(L, 3); break;
        case AT_uint: *((unsigned int*)(&b[offset])) = (unsigned int)lua_tonumber(L, 3); break;
        case AT_long: *((long*)(&b[offset])) = (long)lua_tonumber(L, 3); break;
        case AT_ulong: *((unsigned long*)(&b[offset])) = (unsigned long)lua_tonumber(L, 3); break;
        case AT_ptrdiff_t: *((ptrdiff_t*)(&b[offset])) = (ptrdiff_t)lua_tonumber(L, 3); break;
        case AT_size_t: *((size_t*)(&b[offset])) = (size_t)lua_tonumber(L, 3); break;
        case AT_float: *((float*)(&b[offset])) = (float)lua_tonumber(L, 3); break;
        case AT_double: *((double*)(&b[offset])) = (double)lua_tonumber(L, 3); break;
        case AT_pointer:
                        if(lua_isnil(L, 3) || lua_isuserdata(L, 3)) {
                            *((void**)(&b[offset])) = alien_touserdata(L, 3);
                            break;
                        }
                        /* FALLTHROUGH, hence pointer before string */
        case AT_string: {
                            size_t size;
                            const char *s = lua_tolstring(L, 3, &size);
                            memcpy(*((char**)(&b[offset])), s, size + 1);
                            break;
                        }
        case AT_callback: *((void**)(&b[offset])) = (alien_Function *)alien_checkfunction(L, 3)->fn; break;
        default: return luaL_error(L, "alien: unknown type in buffer:put");
    }
    return 0;
}

int alien_buffer_realloc(lua_State *L) {
    void *aud;
    lua_Alloc lalloc = lua_getallocf(L, &aud);
    alien_Buffer *ab = alien_checkbuffer(L, 1);
    size_t size = luaL_optinteger(L, 2, 1), oldsize = ab->size;
    if (oldsize == 0)
        return luaL_error(L, "alien: buffer to realloc has no size, or non-numeric size");
    ab->p = (char*)lalloc(aud, ab->p, oldsize, size);
    if (size == 0) ab->p = &alien_buffer_empty;
    if (ab->p == NULL) return luaL_error(L, "alien: foo out of memory");
    ab->size = size;
    return 1;
}

int alien_buffer_get(lua_State *L) {
    static const void* funcs[] = {&alien_buffer_tostring,
        &alien_buffer_topointer,
        &alien_buffer_tooffset,
        &alien_buffer_strlen,
        &alien_buffer_get,
        &alien_buffer_set,
        &alien_buffer_realloc};
    static const char *const funcnames[] = { "tostring", "topointer", "tooffset", "strlen", "get", "set", "realloc", NULL };
    char *b = alien_checkbuffer(L, 1)->p;
    if(lua_type(L, 2) == LUA_TSTRING) {
        lua_getuservalue(L, 1);
        if(!lua_isnil(L, -1))
            lua_getfield(L, -1, lua_tostring(L, 2));
        if(lua_isnil(L, -1))
            lua_pushcfunction(L,
                    (lua_CFunction)funcs[luaL_checkoption(L, 2, "tostring", funcnames)]);
    } else {
        void *p;
        ptrdiff_t offset = luaL_checkinteger(L, 2) - 1;
        int type = luaL_checkoption(L, 3, "char", alien_typenames);
        switch(type) {
            case AT_byte: lua_pushnumber(L, (signed char)b[offset]); break;
            case AT_char: lua_pushnumber(L, (unsigned char)b[offset]); break;
            case AT_short: lua_pushnumber(L, *((short*)(&b[offset]))); break;
            case AT_ushort: lua_pushnumber(L, *((unsigned short*)(&b[offset]))); break;
            case AT_int: lua_pushnumber(L, *((int*)(&b[offset]))); break;
            case AT_uint: lua_pushnumber(L, *((unsigned int*)(&b[offset]))); break;
            case AT_long: lua_pushnumber(L, *((long*)(&b[offset]))); break;
            case AT_ulong: lua_pushnumber(L, *((unsigned long*)(&b[offset]))); break;
            case AT_ptrdiff_t: lua_pushnumber(L, *((ptrdiff_t*)(&b[offset]))); break;
            case AT_size_t: lua_pushnumber(L, *((size_t*)(&b[offset]))); break;
            case AT_float: lua_pushnumber(L, *((float*)(&b[offset]))); break;
            case AT_double: lua_pushnumber(L, *((double*)(&b[offset]))); break;
            case AT_string:
                            p = *((void**)&b[offset]);
                            if(p) lua_pushstring(L, (const char *)p); else lua_pushnil(L);
                            break;
            case AT_pointer:
                            p = *((void**)&b[offset]);
                            p ? lua_pushlightuserdata(L, p) : lua_pushnil(L);
                            break;
            case AT_callback:
                            p = *((void**)&b[offset]);
                            p ? alien_makefunction(L, NULL, p, NULL) : lua_pushnil(L);
                            break;
            default:
                            return luaL_error(L, "alien: unknown type in buffer:get");
        }
    }
    return 1;
}

