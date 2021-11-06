#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


struct function_list {
    char** names;
    size_t n;
};

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

function_list* lf_load(const void* vlib)
{
    HMODULE lib = (HMODULE)vlib;
    if (!lib) lib = GetModuleHandle(NULL);
    assert(((PIMAGE_DOS_HEADER)lib)->e_magic == IMAGE_DOS_SIGNATURE);
    PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)((BYTE *)lib + ((PIMAGE_DOS_HEADER)lib)->e_lfanew);
    assert(header->Signature == IMAGE_NT_SIGNATURE);
    assert(header->OptionalHeader.NumberOfRvaAndSizes > 0);
    PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE *)lib + header->
            OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    assert(exports->AddressOfNames != 0);
    BYTE** names = (BYTE**)((int)lib + exports->AddressOfNames);

    function_list* fl = (function_list*)(malloc(sizeof(function_list)));
    fl->n = exports->NumberOfNames;
    fl->names = (char**)malloc(fl->n * sizeof(char*));
    memset(fl->names, 0, fl->n * sizeof(char*));

    for (int i = 0; i < exports->NumberOfNames; i++) {
        const char* str = (char*)((BYTE*)lib + (int)names[i]);
        size_t m = strlen(str);
        fl->names[i] = (char*)malloc(m + 1);
        memcpy(fl->names[i], str, m);
        fl->names[i][m] = '\0';
    }

    return fl;
}
#elif defined(linux)

function_list* lf_load(const void* lib)
{return nullptr;}

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

