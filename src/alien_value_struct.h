#ifndef _ALIEN_VALUE_STRUCT_H_
#define _ALIEN_VALUE_STRUCT_H_

#include "alien_value.h"


class alien_value_struct: public alien_value {
    public:
        alien_value* getMember(const std::string& member);
};

#endif // _ALIEN_VALUE_STRUCT_H_
