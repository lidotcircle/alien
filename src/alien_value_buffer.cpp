#include "alien_value_buffer.h"


alien_value_buffer::alien_value_buffer(alien_type* type, size_t n):
    alien_value(type, nullptr), pptr(nullptr)
{
}

