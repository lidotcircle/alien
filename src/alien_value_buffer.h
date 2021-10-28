#ifndef _ALIEN_VALUE_BUFFER_H_
#define _ALIEN_VALUE_BUFFER_H_

#include "alien.h"
#include "alien_value.h"


class alien_value_buffer: public alien_value {
    private:
        void* pptr;

    public:
        alien_value_buffer(alien_type* type, size_t n);

        uint8_t  get_uint8_t (lua_State* L, size_t one_base_index);
        uint16_t get_uint16_t(lua_State* L, size_t one_base_index);
        uint32_t get_uint32_t(lua_State* L, size_t one_base_index);
        uint64_t get_uint64_t(lua_State* L, size_t one_base_index);
        float    get_float   (lua_State* L, size_t one_base_index);
        double   get_double  (lua_State* L, size_t one_base_index);

        void* ptr() override;
        const void* ptr() const override;

        virtual void assignFrom(const alien_value& val) override;
        virtual void assignFromLua(lua_State* L, size_t idx) override;

        virtual void toLua(lua_State* L) const override;
        virtual alien_value* copy() const override;
};


#endif // _ALIEN_VALUE_BUFFER_H_
