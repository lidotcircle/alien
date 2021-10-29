#ifndef _ALIEN_VALUE_BUFFER_H_
#define _ALIEN_VALUE_BUFFER_H_

#include "alien.h"
#include "alien_value.h"

#define buffer_access_type \
        MENTRY( uint8,  uint8_t  ) \
        MENTRY( uint16, uint16_t ) \
        MENTRY( uint32, uint32_t ) \
        MENTRY( uint64, uint64_t ) \
        MENTRY( sint8,  int8_t   ) \
        MENTRY( sint16, int16_t  ) \
        MENTRY( sint32, int32_t  ) \
        MENTRY( sint64, int64_t  ) \
        MENTRY( float,  float    ) \
        MENTRY( double, double   ) \
        MENTRY( byte,   uint8_t  ) \
        MENTRY( char,   char     )

class alien_value_buffer: public alien_value {
    private:
        size_t buf_len;
        void* pptr;

    public:
        alien_value_buffer(class alien_type* type, size_t n);

#define MENTRY(n, t) t get_##n(lua_State* L, size_t one_base_index, size_t offset) const; \
                     void set_##n(lua_State* L, size_t one_base_index, size_t offset, t val);
        buffer_access_type
#undef MENTRY

        void* ptr() override;
        const void* ptr() const override;

        virtual void assignFrom(const alien_value& val) override;

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        static bool                is_value_buffer(lua_State* L, int idx);
        static alien_value_buffer* check_value_buffer(lua_State* L, int idx);

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);
};

int alien_value_buffer_init(lua_State* L);

#endif // _ALIEN_VALUE_BUFFER_H_
