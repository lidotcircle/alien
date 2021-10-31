#include "alien_type_struct.h"
#include "alien_value_struct.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
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
    std::shared_ptr<size_t> memoffs(new size_t[members.size() + 1], std::default_delete<size_t[]>());

    for(size_t i=0;i<members.size();i++) {
        auto& memtype = members[i];
        this->pffi_type->elements[i] = memtype.second->ffitype();
    }
    this->pffi_type->elements[members.size()] = nullptr;

    ffi_cif cif;
    if (ffi_prep_cif(&cif, this->abi, 0, this->pffi_type, nullptr) != FFI_OK)
        throw std::runtime_error("can't create ffi struct type");

    if (ffi_get_struct_offsets(this->abi, this->pffi_type, memoffs.get()) != FFI_OK)
        throw std::runtime_error("can't get member offsets of ffi struct type");

    for(size_t i=0;i<members.size();i++) {
        auto& mb = members[i];
        if (this->members.find(mb.first) != this->members.end())
            throw std::runtime_error("alien: struct with duplicate member '" + mb.first + "'");
        this->members[mb.first] = std::make_pair(memoffs.get()[i], mb.second);
    }
}

alien_type_struct::~alien_type_struct() {
    delete[] this->pffi_type->elements;
    delete this->pffi_type;
    this->pffi_type = nullptr;
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

std::unique_ptr<alien_type_struct::_member_info> 
    alien_type_struct::member_info(const std::string& member) const 
{
    auto it = this->members.find(member);
    if (it == this->members.end())
        return nullptr;

    auto info = it->second;
    return std::unique_ptr<_member_info>(new _member_info(info.second, info.first));
}

bool alien_type_struct::is_struct() const { return true; }

bool alien_type_struct::is_this_type(lua_State* L, int idx) const {
    return alien_value_struct::is_this_value(this, L, idx);
}

alien_value* alien_type_struct::checkvalue(lua_State* L, int idx) const {
    return alien_value_struct::checkvalue(this, L, idx);
}

