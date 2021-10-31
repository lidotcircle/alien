#include "alien_value_callback.h"
#include "alien_function.h"
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

int alien_value_callback_new(lua_State* L) {
    if (lua_gettop(L) != 3)
        return luaL_error(L, "alien_value_callback_new: invalid arguments");

    lua_pushcfunction(L, alien_value_callback_new__);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_call(L, 2, 1);
    return 1;
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

int alien_value_callback_new__(lua_State* L) {
    if (!lua_isfunction(L, 1) || !lua_istable(L, 2))
        return luaL_error(L, "alien_value_callback_new: invalid arguments");

    ffi_abi abi = FFI_DEFAULT_ABI;
    alien_type* rettype = nullptr;
    vector<alien_type*> params;

    std::tie(abi, rettype, params) = alien_function__parse_types_table(L, 2);

    auto cbtype = alien_type_byname(L, "callback");
    alien_value_callback cb(cbtype, L, 1, abi, rettype, params);
    cb.to_lua(L);
    return 1;
}

alien_value_callback::alien_value_callback(const alien_type* type,
        lua_State* L, int funcidx,
        ffi_abi abi, alien_type* ret, const vector<alien_type*>& params):
    alien_value(type), pcallback_info(std::make_shared<callback_info>())
{
    auto ci = this->pcallback_info;
    ci->L = L;
    ci->closure = (ffi_closure*)ffi_closure_alloc(sizeof(ffi_closure), &ci->ffi_codeloc);
    ci->abi = abi;
    ci->ret_type = ret;
    ci->params = params;
    ci->lfunc_ref = LUA_NOREF;
    if (!ci->params.empty()) {
        ci->ffi_params = 
            std::shared_ptr<ffi_type*>(new ffi_type*[ci->params.size()], [](ffi_type** ptr) { delete[] ptr; });

        for(size_t i=0;i<ci->params.size();i++)
            ci->ffi_params.get()[i] = ci->params[i]->ffitype();
    }

    ffi_status status = 
        ffi_prep_cif(&ci->cif, abi, ci->params.size(),
                     ci->ret_type->ffitype(), ci->ffi_params.get());

    if (status == FFI_OK)
        status = ffi_prep_closure_loc(ci->closure, &ci->cif,
                                      &alien_value_callback::callback_call, 
                                      this->pcallback_info.get(), ci->ffi_codeloc);

    if (status != FFI_OK) {
        ffi_closure_free(ci->closure);
        ci->closure = nullptr;
        ci->ffi_codeloc = nullptr;
        throw std::runtime_error("alien: can't create callback");
    }

    lua_pushvalue(L, funcidx);
    ci->lfunc_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    *static_cast<void**>(this->ptr()) = ci->ffi_codeloc;
}

alien_value_callback::alien_value_callback(const alien_value_callback& other):
    alien_value(other.alientype()), pcallback_info(other.pcallback_info)
{
    *static_cast<void**>(this->ptr()) = this->pcallback_info->ffi_codeloc;
}

void* alien_value_callback::funcaddr() const {
    return this->pcallback_info->ffi_codeloc;
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
    *pvc = new alien_value_callback(*this);
}
alien_value* alien_value_callback::copy() const {
    return new alien_value_callback(*this);
}

alien_value_callback::~alien_value_callback() {}
alien_value_callback::callback_info::~callback_info() {
    if (this->closure)
        ffi_closure_free(this->closure);
    luaL_unref(L, LUA_REGISTRYINDEX, lfunc_ref);
}

/** static */
void alien_value_callback::callback_call(ffi_cif* cif, void *resp, void **args, void* data) {
    auto vc = static_cast<callback_info*>(data);
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

/** implement from_lua from_ptr and new_value static function */
alien_value* alien_value_callback::from_lua(const alien_type* type, lua_State* L, int idx) {
    alien_value_callback* cb = alien_checkcallback(L, idx);
    return cb->copy();
}
alien_value* alien_value_callback::from_ptr(const alien_type* type, lua_State* L, void* ptr) {
    throw std::runtime_error("alien: can't create callback from ptr");
}
alien_value* alien_value_callback::from_shr(const alien_type* type, lua_State* L, std::shared_ptr<char> mem, void* ptr) {
    throw std::runtime_error("alien: can't create callback");
}
alien_value* alien_value_callback::new_value(const alien_type* type, lua_State* L) {
    throw std::runtime_error("alien: can't create callback");
}

/** static */
bool alien_value_callback::is_this_value(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_callback());
    return alien_iscallback(L, idx);
}

/** static */
alien_value_callback* alien_value_callback::checkvalue(const alien_type* type, lua_State* L, int idx) {
    assert(type->is_callback());
    return alien_checkcallback(L, idx);
}

