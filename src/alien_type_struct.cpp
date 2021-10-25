#include "alien_type_struct.h"
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;


alien_type_struct::alien_type_struct(ffi_abi abi, const vector<pair<string, alien_type*>>& members): alien_type(abi, nullptr) {
    this->pffi_type = new ffi_type();
    this->pffi_type->type = FFI_TYPE_STRUCT;
    this->pffi_type->elements = new ffi_type*[members.size() + 1];
    this->pffi_type->size = 0;
    this->pffi_type->alignment = 0;
    this->member_offs = new size_t[this->members.size() + 1];

    for(size_t i=0;i<members.size();i++) {
        auto& memtype = members[i];
        this->pffi_type->elements[i] = memtype.second->pffi_type;
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

size_t alien_type_struct::__offsetof(const std::string& member) {
    int n = -1;
    for (size_t i=0;i<this->members.size();i++) {
        if (member == this->members[i].first) {
            n = i;
            break;
        }
    }

    return this->member_offs[n];
}
