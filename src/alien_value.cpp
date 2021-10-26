#include "alien_value.h"
#include <memory>
#include <string.h>
using namespace std;

#define ALIEN_VALUE_STRUCT_META  "alien_value_struct_meta"
#define ALIEN_VALUE_UNION_META   "alien_value_union_meta"
#define ALIEN_VALUE_BUFFER_META  "alien_value_buffer_meta"
#define ALIEN_VALUE_POINTER_META "alien_value_pointer_meta"
#define ALIEN_VALUE_REF_META     "alien_value_ref_meta"

/** operator dispatcher */
int alien_operator_method_new(lua_State* L, alien_type* type) {
    if (type->is_basic()) {
    } else if (type->is_struct()) {
    } else if (type->is_union()) {
    }

    return 1;
}

alien_value::alien_value(const alien_type* type):
    type(type), _mem(new char[type->__sizeof()], [](char ptr[]) {delete[] ptr;}),
    val_ptr(_mem.get()) {}

alien_value::alien_value(const alien_type* type, void* ptr):
    type(type), _mem(static_cast<char*>(ptr), [](char ptr[]) {}),
    val_ptr(_mem.get()) {}

void* alien_value::ptr() { return this->val_ptr; }
const void* alien_value::ptr() const { return this->val_ptr; }
size_t alien_value::__sizeof() const { return this->type->__sizeof(); }

void alien_value::assignFrom(const alien_value& val) {
    if (this->type != val.type)
        throw std::runtime_error("alien: assignment between incompatible type");
        return;

    memcpy(this->ptr(), const_cast<void*>(val.ptr()), this->__sizeof());
}

void alien_value::assignFromLua(lua_State* L, size_t idx) {
    auto v = this->type->fromLua(L, idx);
    this->assignFrom(*v);
    delete v;
}

