#include "alien_type_union.h"
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;


alien_type_union::alien_type_union(ffi_abi abi, const vector<pair<string, alien_type*>>& members): alien_type(abi, nullptr) {
    this->pffi_type = new ffi_type();
    this->pffi_type->type = FFI_TYPE_STRUCT;
    this->pffi_type->size = 0;
    this->pffi_type->alignment = 0;
    this->pffi_type->elements = new ffi_type*[2];

    for(auto& m: members) {
        if (m.second->pffi_type->size > this->pffi_type->size) {
            this->pffi_type->size = m.second->pffi_type->size;
        }
        if (m.second->pffi_type->alignment > this->pffi_type->alignment) {
            this->pffi_type->alignment = m.second->pffi_type->alignment;
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
            return m.second->pffi_type->size;
    }

    return 0;
}
