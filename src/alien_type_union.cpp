#include "alien_type_union.h"
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;


alien_type_union::alien_type_union(const std::string& t, ffi_abi abi, const vector<pair<string, alien_type*>>& members): alien_type(t, abi, nullptr) {
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
    }

    this->pffi_type->elements[1] = nullptr;
    this->members = members;
}

alien_type_union::~alien_type_union() {
    delete[] this->pffi_type->elements;
    delete this->pffi_type;
    this->pffi_type = nullptr;
}

size_t alien_type_union::sizeof_member(const std::string& member) {
    for (auto& m: this->members) {
        if (m.first == member)
            return m.second->ffitype()->size;
    }

    return 0;
}
