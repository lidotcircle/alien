#include "alien_value.h"
#include <memory>
#include <string.h>

#define ALIEN_VALUE_META "alien_value_meta"


int alien_value_from_type(lua_State* L, alien_type* type) {
    alien_value* val = new alien_value(type);

    return 1;
}

alien_value::alien_value(const alien_type* type):
    type(type), _mem(new char[type->__sizeof()], [](char ptr[]) {delete[] ptr;}),
    val_ptr(_mem.get()) {}

void* alien_value::ptr() { return this->val_ptr; }
size_t alien_value::__sizeof() { return this->type->__sizeof(); }

