#ifndef _ALIEN_VALUE_BUFFER_H_
#define _ALIEN_VALUE_BUFFER_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_buffer: public alien_value {
    public:
        uint8_t  get_uint8_t (lua_State* L, size_t one_base_index);
        uint16_t get_uint16_t(lua_State* L, size_t one_base_index);
        uint32_t get_uint32_t(lua_State* L, size_t one_base_index);
        uint64_t get_uint64_t(lua_State* L, size_t one_base_index);
        float    get_float   (lua_State* L, size_t one_base_index);
        double   get_double  (lua_State* L, size_t one_base_index);
};


#endif // _ALIEN_VALUE_BUFFER_H_
