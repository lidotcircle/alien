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
#include <winnt.h>

function_list* lf_load(const void* lib)
{
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
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>

function_list* lf_load(const void* lib)
{
    struct link_map* mlib = (struct link_map*)lib;
    /*
    if (!dlinfo(lib, RTLD_DI_LINKMAP, &mlib)) {
        return NULL;
    }
    */

    Elf64_Sym* symtab = NULL;
    char* strtab = NULL;
    int symentries = 0;
    for (Elf64_Dyn* section = mlib->l_ld; section->d_tag != DT_NULL; ++section)
    {
        if (section->d_tag == DT_SYMTAB)
            symtab = (Elf64_Sym *)section->d_un.d_ptr;
        if (section->d_tag == DT_STRTAB)
            strtab = (char*)section->d_un.d_ptr;
        if (section->d_tag == DT_SYMENT)
            symentries = section->d_un.d_val;
    }

    size_t size = strtab - (char *)symtab;
    size_t esize = 0;
    for (size_t k = 0; k < size / symentries; ++k) {
        if (ELF64_ST_TYPE(symtab[k].st_info) == STT_FUNC)
            esize += 1;
    }

    function_list* fl = (function_list*)(malloc(sizeof(function_list)));
    fl->n = esize;
    fl->names = (char**)malloc(fl->n * sizeof(char*));
    memset(fl->names, 0, fl->n * sizeof(char*));
    size_t nt = 0;

    for (size_t k = 0; k < size / symentries; ++k)
    {
        Elf64_Sym* sym = &symtab[k];
        // If sym is function
        if (ELF64_ST_TYPE(symtab[k].st_info) == STT_FUNC)
        {
            //str is name of each symbol
            const char* str = &strtab[sym->st_name];
            size_t m = strlen(str);
            char* mstr = (char*)malloc(m + 1);
            memcpy(mstr, str, m);
            mstr[m] = '\0';
            fl->names[nt] = mstr;
            nt++;
        }
    }
    assert(nt == esize);
    return fl;
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

