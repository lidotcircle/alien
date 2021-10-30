#ifndef _ALIEN_VALUE_REF_H_
#define _ALIEN_VALUE_REF_H_

#include "alien.h"
#include "alien_value.h"
#include <memory>


class alien_value_ref: public alien_value {
    private:
        struct ref_info {
            alien_value* aval;
            alien_type* ref_type;
            std::shared_ptr<char> __uptr;
            void* ref_ptr;

            ~ref_info();
        };
        std::shared_ptr<ref_info> pref_info;
        void init_value();

    public:
        alien_value_ref(const alien_type* type);
        alien_value_ref(const alien_type* type, std::shared_ptr<char> mem, void* ptr);
        alien_value_ref(const alien_value_ref& other);

        virtual void to_lua(lua_State* L) const override;
        virtual alien_value* copy() const override;

        alien_value* access_member(lua_State* L, const std::string&) const;

        virtual ~alien_value_ref() override;

        static alien_value* from_lua(const alien_type* type, lua_State* L, int idx);
        static alien_value* from_ptr(const alien_type* type, lua_State* L, void* ptr);
        static alien_value* new_value(const alien_type* type, lua_State* L);

        static bool is_this_value(const alien_type* type, lua_State* L, int idx);
        static alien_value_ref* checkvalue(const alien_type* type, lua_State* L, int idx);
};

#endif // _ALIEN_VALUE_REF_H_
