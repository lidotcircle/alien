#include "alien_type_basic.h"

#define MAX_ab(a, b) ((a) > (b) ? (a) : (b))


alien_type::alien_type(const std::string& t, ffi_abi abi, ffi_type* type): type_name(t), abi(abi), pffi_type(type) {} 

ffi_type* alien_type::ffitype() { return this->pffi_type; }

size_t alien_type::__sizeof() const { return MAX_ab(this->pffi_type->size, this->pffi_type->alignment); }

const std::string& alien_type::__typename() const { return this->type_name; }

alien_type::~alien_type() { }
