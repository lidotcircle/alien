#include "alien_value_callback.h"
#include <vector>
#include <string>
#include <stdexcept>
using namespace std;


alien_value_callback::alien_value_callback(const alien_type* type,
        lua_State* L, int funcidx,
        ffi_abi abi, alien_type* ret, const vector<alien_type*>& params):
    alien_value(type, nullptr), L(L),
    closure(nullptr), ffi_codeloc(nullptr), ffi_params(),
    abi(abi), ret_type(ret), params(params), lfunc_ref(LUA_NOREF)
{
    this->closure = (ffi_closure*)ffi_closure_alloc(sizeof(ffi_closure), &this->ffi_codeloc);

    ffi_status status = 
        ffi_prep_cif(&this->cif, abi, this->params.size(),
                     this->ret_type->ffitype(), this->ffi_params.get());

    ffi_params = std::shared_ptr<ffi_type*>(new ffi_type*[this->params.size()], [](ffi_type** ptr) { delete[] ptr; });
    for(size_t i=0;i<this->params.size();i++)
        ffi_params.get()[i] = this->params[i]->ffitype();

    if (status == FFI_OK)
        status = ffi_prep_closure_loc(this->closure, &this->cif, &alien_value_callback::callback_call, this, this->ffi_codeloc);

    if (status != FFI_OK) {
        ffi_closure_free(this->closure);
        this->closure = nullptr;
        this->ffi_codeloc = nullptr;
        throw std::runtime_error("alien: can't create callback");
    }

    lua_pushvalue(L, funcidx);
    this->lfunc_ref = luaL_ref(L, LUA_REGISTRYINDEX);
}

void* alien_value_callback::ptr() {
    return this->ffi_codeloc;
}

const void* alien_value_callback::ptr() const {
    return this->ffi_codeloc;
}

void alien_value_callback::assignFrom(const alien_value& val) {
    throw std::runtime_error("alien: can't change callback");
}
void alien_value_callback::assignFromLua(lua_State* L, size_t idx) {
    luaL_error(L, "alien: can't change callback");
}
void alien_value_callback::toLua(lua_State* L) const {
}
alien_value* alien_value_callback::copy() const {
    throw std::runtime_error("alien: can't copy callback");
}

alien_value_callback::~alien_value_callback() {
}

/** static */
void alien_value_callback::callback_call(ffi_cif* cif, void *resp, void **args, void* data) {
}

