#include "alien_type_union.h"
#include "alien_value_union.h"
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;


alien_type_union::alien_type_union(const std::string& t, 
                                   ffi_abi abi, 
                                   const vector<pair<string, alien_type*>>& members):
    alien_type(t), abi(abi) 
{
    this->pffi_type = new ffi_type();
    this->pffi_type->type = FFI_TYPE_STRUCT;
    this->pffi_type->size = 0;
    this->pffi_type->alignment = 0;
    this->pffi_type->elements = new ffi_type*[2];

    for(auto& m: members) {
        if (m.second->ffitype()->size > this->pffi_type->size) {
            this->pffi_type->size = m.second->ffitype()->size;
        }
        if (m.second->ffitype()->alignment > this->pffi_type->alignment) {
            this->pffi_type->alignment = m.second->ffitype()->alignment;
        }

        if (this->member_map.find(m.first) != this->member_map.end())
            throw runtime_error("duplicate member name: " + m.first);

        this->member_map[m.first] = m.second;
    }

    this->pffi_type->elements[1] = nullptr;
}

alien_type_union::~alien_type_union() {
    delete[] this->pffi_type->elements;
    delete this->pffi_type;
    this->pffi_type = nullptr;
}

ffi_type* alien_type_union::ffitype() {
    return this->pffi_type;
}

alien_value* alien_type_union::from_lua(lua_State* L, int idx) const {
    return alien_value_union::from_lua(this, L, idx);
}

alien_value* alien_type_union::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_union::from_ptr(this, L, ptr);
}

alien_value* alien_type_union::new_value(lua_State* L) const {
    return alien_value_union::new_value(this, L);
}

const alien_type* alien_type_union::member_type(const string& member) const
{
    auto fm = this->member_map.find(member);
    if (fm == this->member_map.end())
        return nullptr;

    return fm->second;
}

bool alien_type_union::is_union() const { return true; }

bool alien_type_union::is_this_type(lua_State* L, int idx) const {
    return alien_value_union::is_this_value(this, L, idx);
}

alien_value* alien_type_union::checkvalue(lua_State* L, int idx) const {
    return alien_value_union::checkvalue(this, L, idx);
}

