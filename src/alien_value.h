#ifndef _ALIEN_VALUE_H_
#define _ALIEN_VALUE_H_

#include <memory>
#include <string>
#include "alien.h"
#include "alien_type_basic.h"

/* operator dispatcher
 * create new lua value and push to stack */
int alien_operator_method_new(lua_State* L, alien_type* type);

/* forward declaration */
class alien_value_ref;
class alien_type;


class alien_value {
    protected:
        const alien_type* type;
        std::shared_ptr<char> _mem;
        void* val_ptr;
        friend class alien_type;
        friend class alien_value_ref;

    public:
        alien_value() = delete;
        alien_value(const alien_type* type);
        alien_value(const alien_type* type, std::shared_ptr<char> mem, void* ptr);

        /** pass pointer to ffi call */
        virtual void* ptr();
        virtual const void* ptr() const;
        size_t __sizeof() const;
        const alien_type* alientype() const;

        virtual void assignFrom(const alien_value& val);
        virtual void assignFromLua(lua_State* L, size_t idx);

        virtual void to_lua(lua_State* L) const = 0;
        virtual alien_value* copy() const = 0;
        virtual ~alien_value() = default;
};


int alien_value_init(lua_State* L);

#endif // _ALIEN_VALUE_H_
