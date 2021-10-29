#include "alien_value_callback.h"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <string.h>
#include <assert.h>
using namespace std;

#define ALIEN_VALUE_CALLBACK_META "alien_value_callback_meta"


static int alien_value_callback_gc(lua_State* L);
static int alien_value_callback_tostring(lua_State* L);
static int alien_value_callback_addr(lua_State* L);

int alien_value_callback_init(lua_State* L) {
    luaL_newmetatable(L, ALIEN_VALUE_CALLBACK_META);

    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, alien_value_callback_gc);
    lua_settable(L, -3);

    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, alien_value_callback_tostring);
    lua_settable(L, -3);

    lua_pushliteral(L, "__index");
    lua_newtable(L);
    lua_pushliteral(L, "addr");
    lua_pushcfunction(L, alien_value_callback_addr);
    lua_settable(L, -3);
    lua_settable(L, -3);

    lua_pop(L, 1);
    return 0;
}

static bool alien_iscallback(lua_State* L, int idx) {
    return luaL_testudata(L, idx, ALIEN_VALUE_CALLBACK_META) != nullptr;
}
static alien_value_callback* alien_checkcallback(lua_State* L, int idx) {
    alien_value_callback** pvc = (alien_value_callback**)luaL_testudata(L, idx, ALIEN_VALUE_CALLBACK_META);
    return *pvc;
}

static int alien_value_callback_gc(lua_State* L) {
    auto vc = alien_checkcallback(L, 1);
    vc->ref_dec();
    if (vc->noref())
        delete vc;
    return 0;
}

static int alien_value_callback_tostring(lua_State* L) {
    auto vc = alien_checkcallback(L, 1);
    lua_pushstring(L, "[alien callback]");
    return 1;
}

static int alien_value_callback_addr(lua_State* L) {
    auto vc = alien_checkcallback(L, 1);
    lua_pushinteger(L, reinterpret_cast<ptrdiff_t>(vc->ptr()));
    return 1;
}

alien_value_callback::alien_value_callback(const alien_type* type,
        lua_State* L, int funcidx,
        ffi_abi abi, alien_type* ret, const vector<alien_type*>& params):
    alien_value(type, nullptr), L(L), ref_count(0),
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

void alien_value_callback::ref_dec() const {
    if (this->ref_count == 0)
        throw std::runtime_error("aline: fatal error, decrease reference "
                                 "count of a zero reference alien callback");

    const_cast<alien_value_callback*>(this)->ref_count--;
}

void alien_value_callback::ref_inc() const {
    const_cast<alien_value_callback*>(this)->ref_count++;
}

bool alien_value_callback::noref() const {
    return this->ref_count == 0;
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
void alien_value_callback::to_lua(lua_State* L) const {
    const alien_value_callback** pvc = (const alien_value_callback**)lua_newuserdata(L, sizeof(alien_value_callback*));
    luaL_setmetatable(L, ALIEN_VALUE_CALLBACK_META);
    this->ref_inc();
    *pvc = this;
}
alien_value* alien_value_callback::copy() const {
    throw std::runtime_error("alien: can't copy callback");
}

alien_value_callback::~alien_value_callback() {
    if (this->closure != nullptr) {
        ffi_closure_free(this->closure);
        this->closure = nullptr;
    }
}

/** static */
void alien_value_callback::callback_call(ffi_cif* cif, void *resp, void **args, void* data) {
    alien_value_callback* vc = static_cast<alien_value_callback*>(data);
    lua_State* L = vc->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, vc->lfunc_ref);
    for(size_t i=0;i<vc->params.size();i++) {
        auto p = vc->params[i];
        std::shared_ptr<alien_value> ap(p->from_ptr(L, args[i]));
        ap->to_lua(L);
    }

    lua_call(L, vc->params.size(), 1);

    if (!vc->ret_type->is_void()) {
        assert(resp != nullptr);
        std::shared_ptr<alien_value> retval(vc->ret_type->from_lua(L, -1));
        memcpy(resp, retval->ptr(), retval->__sizeof());
    }

    lua_pop(L, 1);
    return;
}

