#include "alien_exception.h"
#include <string>
using namespace std;

AlienException::AlienException(const string& msg) : msg("alien: " + msg) {}

const char* AlienException::what() const throw() {
    return msg.c_str();
}

