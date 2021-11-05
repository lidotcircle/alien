#ifndef _ALIEN_EXCEPTION_H_
#define _ALIEN_EXCEPTION_H_

#include <exception>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __GNUC__
#define printf_check(fmt, vara) __attribute__((format(printf, fmt, vara)))
#else
#define printf_check(fmt, vara)
#endif

class AlienException : public std::exception {
    protected:
        std::string msg;

    public:
        AlienException() = default;
        AlienException(const char* fmt, ...) printf_check(2, 3)
        {
            char buf[4096];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);
            this->msg = buf;
        }

        virtual const char* what() const throw() override;
};


#define alien_exception_list \
    MENTRY( AlienNotImplementedException,  AlienException) \
    MENTRY( AlienBadTypeException,         AlienException) \
    MENTRY( AlienInvalidValueException,    AlienException) \
    MENTRY( AlienOutOfRange,               AlienException) \
    MENTRY( AlienLuaThrow,                 AlienException) \
    MENTRY( AlienFatalError,               AlienException) \
    MENTRY( AlienInvalidArgumentException, AlienException)

#define MENTRY(new_exception, base) \
    class new_exception: public base { \
        public: \
            new_exception(const char* fmt, ...) printf_check(2, 3) {\
                char buf[4096]; \
                va_list args; \
                va_start(args, fmt); \
                vsnprintf(buf, sizeof(buf), fmt, args); \
                va_end(args); \
                this->msg = buf; \
            } \
    };
alien_exception_list
#undef MENTRY


extern char alien_exception_buffer[4096];
#define ALIEN_EXCEPTION_BEGIN() \
    try {
#define ALIEN_EXCEPTION_END() \
    } catch (AlienException& e) { \
        auto n = dynamic_cast<AlienFatalError*>(&e); \
        if (n != nullptr) \
            throw e; \
        strncpy(alien_exception_buffer, e.what(), sizeof(alien_exception_buffer)); \
    } \
    return luaL_error(L, alien_exception_buffer);

#endif // _ALIEN_EXCEPTION_H_
