#include "alien_exception.h"
#include <string>
#include <stdio.h>
using namespace std;


const char* AlienException::what() const throw() {
    return msg.c_str();
}
