#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


struct function_list {
    char** names;
    size_t n;
};

#if defined(WINDOWS)
function_list* lf_load(const void* lib)
{
    fprintf(stderr, "not implemented\n");
    exit(1);
}
#elif defined(linux)
function_list* lf_load(const void* lib)
{
    return NULL;
}
#else
function_list* lf_load(const void* lib)
{
    fprintf(stderr, "not implemented\n");
    exit(1);
}
#endif

void lf_free(function_list* flist) {
    if (flist->names != NULL) {
        assert(flist->n > 0);
        for(size_t i=0;i<flist->n;i++)
            free(flist->names[i]);
        flist->n = 0;

        free(flist->names);
        flist->names = NULL;
    }

    free(flist);
}

size_t lf_size(function_list* flist) {
    return flist->n;
}

const char* lf_index(function_list* flist, size_t n) {
    assert(n < flist->n);
    return flist->names[n];
}

