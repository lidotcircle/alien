#ifndef ALIEN_UTILS_H
#define ALIEN_UTILS_H

#include <stdlib.h>


struct function_list;
typedef struct function_list function_list;

function_list* lf_load(const void* lib);
void           lf_free(function_list* flist);
size_t         lf_size(function_list* flist);
const char*    lf_index(function_list* flist, size_t n);


#endif // ALIEN_UTILS_H
