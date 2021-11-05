#include "alien_exception.h"
#include <string>
#include <stdio.h>
using namespace std;


const char* AlienException::what() const throw() {
    return msg.c_str();
}

char alien_exception_buffer[4096];

