#ifndef _ALIEN_VALUE_CALLBACK_H_
#define _ALIEN_VALUE_CALLBACK_H_

#include "alien.h"
#include "alien_value.h"
#include <ffi.h>
#include <vector>
#include <memory>


class alien_value_callback: alien_value {
    private:
        ffi_closure* closure;
        void* ffi_codeloc;
        lua_State* L;
        ffi_abi abi;
        ffi_cif cif;
        alien_type* ret_type;
        std::vector<alien_type*> params;
        int  lfunc_ref;
        std::shared_ptr<ffi_type*> ffi_params;

    public:
        alien_value_callback(const alien_type* type,
                             lua_State* L, int funcidx,
                             ffi_abi abi, alien_type* ret, const std::vector<alien_type*>& params);

        virtual void* ptr() override;
        virtual const void* ptr() const override;

        virtual void assignFrom(const alien_value& val) override;
        virtual void assignFromLua(lua_State* L, size_t idx) override;

        virtual void toLua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        virtual ~alien_value_callback() override;

        static void callback_call(ffi_cif* cif, void *resp, void **args, void* data);
};

#endif // _ALIEN_VALUE_CALLBACK_H_
