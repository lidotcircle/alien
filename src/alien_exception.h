#ifndef _ALIEN_EXCEPTION_H_
#define _ALIEN_EXCEPTION_H_

#include <exception>
#include <string>

class AlienException : public std::exception {
    private:
        const std::string msg;

    public:
        AlienException(const std::string& msg);
        virtual const char* what() const throw() override;
};

#endif // _ALIEN_EXCEPTION_H_
