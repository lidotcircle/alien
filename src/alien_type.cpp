#include "alien_type.h"

#define MAX_ab(a, b) ((a) > (b) ? (a) : (b))


alien_type::alien_type(ffi_abi abi, ffi_type* type): abi(abi), pffi_type(type) {} 

size_t alien_type::__sizeof() { return MAX_ab(this->pffi_type->size, this->pffi_type->alignment); }

alien_type::~alien_type() { }
