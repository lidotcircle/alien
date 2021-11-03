#include "alien_type_array.h"
#include "alien_value_array.h"
#include <string>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <memory>
using namespace std;


alien_type_array::alien_type_array(const alien_type* elemtype, size_t len, ffi_abi abi):
    alien_type(elemtype->__typename() + "[" + std::to_string(len) +  "]"),
    abi(abi), elem_type(elemtype), len(len), elem_aligned_size(0)
{
    assert(len > 0);
    this->pffi_type = new ffi_type();
    this->pffi_type->type = FFI_TYPE_STRUCT;
    this->pffi_type->elements = new ffi_type*[len + 1];
    this->pffi_type->size = 0;
    this->pffi_type->alignment = 0;
    auto elemffitype = const_cast<ffi_type*>(this->elem_type->ffitype());

    for(size_t i=0;i<len;i++)
        this->pffi_type->elements[i] = elemffitype;
    this->pffi_type->elements[len] = nullptr;

    ffi_cif cif;
    if (ffi_prep_cif(&cif, this->abi, 0, this->pffi_type, nullptr) != FFI_OK)
        throw std::runtime_error("can't create ffi array type");

    ffi_cif kcif;
    ffi_type stype;
    stype.type = FFI_TYPE_STRUCT;
    stype.alignment = 0;
    stype.size = 0;
    std::shared_ptr<ffi_type*> et(new ffi_type*[3], std::default_delete<ffi_type*[]>());
    stype.elements = et.get();
    stype.elements[0] = elemffitype;
    stype.elements[1] = elemffitype;
    stype.elements[2] = nullptr;
    if (ffi_prep_cif(&kcif, this->abi, 0, this->pffi_type, nullptr) != FFI_OK)
        throw std::runtime_error("can't create ffi array type");

    size_t offs[2];
    if (ffi_get_struct_offsets(abi, &stype, offs) != FFI_OK)
        throw std::runtime_error("alien: can't get offsets, create array type failed");
    this->elem_aligned_size = offs[1];
}

alien_type_array::~alien_type_array() {
    delete[] this->pffi_type->elements;
    delete this->pffi_type;
    this->pffi_type = nullptr;
}

ffi_type* alien_type_array::ffitype() {
    return this->pffi_type;
}

alien_value* alien_type_array::from_lua(lua_State* L, int idx) const {
    return alien_value_array::from_lua(this, L, idx);
}

alien_value* alien_type_array::from_ptr(lua_State* L, void* ptr) const {
    return alien_value_array::from_ptr(this, L, ptr);
}

alien_value* alien_type_array::from_shr(lua_State* L, std::shared_ptr<char> m, void* ptr) const {
    return alien_value_array::from_shr(this, L, m, ptr);
}

alien_value* alien_type_array::new_value(lua_State* L) const {
    return alien_value_array::new_value(this, L);
}

size_t alien_type_array::array_length() const { return this->len; }

size_t alien_type_array::element_size() const { return this->elem_type->__sizeof(); }

size_t alien_type_array::element_aligned_size() const { return this->elem_aligned_size; }

const alien_type* alien_type_array::elemtype() const { return this->elem_type; }

bool alien_type_array::is_array() const { return true; }

bool alien_type_array::is_this_type(lua_State* L, int idx) const {
    return alien_value_array::is_this_value(this, L, idx);
}

alien_value* alien_type_array::checkvalue(lua_State* L, int idx) const {
    return alien_value_array::checkvalue(this, L, idx);
}

