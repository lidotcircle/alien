#ifndef _ALIEN_VALUE_H_
#define _ALIEN_VALUE_H_

#include <memory>
#include <string>
#include "alien.h"
#include "alien_type_basic.h"

/* operator dispatcher
 * create new lua value and push to stack */
int alien_operator_method_new(lua_State* L, alien_type* type);


class alien_value {
    protected:
        const alien_type* type;
        std::shared_ptr<char[]> _mem;
        void* val_ptr;

    public:
        alien_value() = delete;
        alien_value(const alien_type* type);
        alien_value(const alien_type* type, void* ptr);

        /** pass pointer to ffi call */
        void* ptr();
        const void* ptr() const;
        size_t __sizeof() const;

        virtual void assignFrom(const alien_value& val);
        virtual void assignFromLua(lua_State* L, size_t idx);

        virtual void toLua(lua_State* L) const = 0;
        virtual alien_value* copy() const = 0;
        virtual ~alien_value() = default;
};

#endif // _ALIEN_VALUE_H_
