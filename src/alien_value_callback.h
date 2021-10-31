#ifndef _ALIEN_VALUE_CALLBACK_H_
#define _ALIEN_VALUE_CALLBACK_H_

#include "alien.h"
#include "alien_value.h"
#include <ffi.h>
#include <vector>
#include <memory>


class alien_value_callback: public alien_value {
    private:
        struct callback_info {
            ffi_closure* closure;
            void* ffi_codeloc;
            lua_State* L;
            ffi_abi abi;
            ffi_cif cif;
            alien_type* ret_type;
            std::vector<alien_type*> params;
            int  lfunc_ref;
            std::shared_ptr<ffi_type*> ffi_params;

            ~callback_info();
        };
        std::shared_ptr<callback_info> pcallback_info;

    public:
        alien_value_callback(const alien_type* type,
                             lua_State* L, int funcidx,
                             ffi_abi abi, alien_type* ret, const std::vector<alien_type*>& params);
        alien_value_callback(const alien_value_callback& other);

        virtual void assignFrom(const alien_value& val) override;
        virtual void assignFromLua(lua_State* L, size_t idx) override;

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        virtual ~alien_value_callback() override;

        static void callback_call(ffi_cif* cif, void *resp, void **args, void* data);

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_callback* checkvalue(const alien_type* type, lua_State* L, int idx);
};

int alien_value_callback_init(lua_State* L);
int alien_value_callback_new(lua_State* L);

#endif // _ALIEN_VALUE_CALLBACK_H_
