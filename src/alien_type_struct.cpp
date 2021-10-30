#include "alien_type_struct.h"
#include "alien_value_struct.h"
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;


alien_type_struct::alien_type_struct(const std::string& t,
                                     ffi_abi abi,
                                     const vector<pair<string, alien_type*>>& members):
    alien_type(t), abi(abi)
{
    this->pffi_type = new ffi_type();
    this->pffi_type->type = FFI_TYPE_STRUCT;
    this->pffi_type->elements = new ffi_type*[members.size() + 1];
    this->pffi_type->size = 0;
    this->pffi_type->alignment = 0;
    this->member_offs = new size_t[this->members.size() + 1];

    for(size_t i=0;i<members.size();i++) {
        auto& memtype = members[i];
        this->pffi_type->elements[i] = memtype.second->ffitype();
    }
    this->pffi_type->elements[members.size()] = nullptr;

    ffi_cif cif;
    if (ffi_prep_cif(&cif, this->abi, 0, this->pffi_type, nullptr) != FFI_OK)
        throw std::runtime_error("can't create ffi struct type");

    if (ffi_get_struct_offsets(this->abi, this->pffi_type, this->member_offs) != FFI_OK)
        throw std::runtime_error("can't get member offsets of ffi struct type");

    this->members = members;
}

alien_type_struct::~alien_type_struct() {
    delete[] this->pffi_type->elements;
    delete this->pffi_type;
    this->pffi_type = nullptr;

    delete[] this->member_offs;
    this->member_offs = nullptr;
}

bool alien_type_struct::has_member(const std::string& member) {
    for (auto& m: this->members) {
        if (m.first == member)
            return true;
    }

    return false;
}

ffi_type* alien_type_struct::ffitype() {
    return this->pffi_type;
}

alien_value* alien_type_struct::from_lua(lua_State* L, int idx) const {
    return alien_value_struct::from_lua(this, L, idx);
}

alien_value* alien_type_struct::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_struct::from_ptr(this, L, ptr);
}

alien_value* alien_type_struct::new_value(lua_State* L) const {
    return alien_value_struct::new_value(this, L);
}

size_t alien_type_struct::__offsetof(const std::string& member) {
    int n = -1;
    for (size_t i=0;i<this->members.size();i++) {
        if (member == this->members[i].first) {
            n = i;
            break;
        }
    }

    if (n == -1)
        throw std::runtime_error("alien: can't find offset of '" + member + "'");

    return this->member_offs[n];
}

bool alien_type_struct::is_struct() const { return true; }

bool alien_type_struct::is_this_type(lua_State* L, int idx) const {
    return alien_value_struct::is_this_value(this, L, idx);
}

alien_value* alien_type_struct::checkvalue(lua_State* L, int idx) const {
    return alien_value_struct::checkvalue(this, L, idx);
}

