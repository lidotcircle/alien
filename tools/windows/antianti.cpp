#include <funchook.h>
#include <Windows.h>
#include <stdio.h>

#define EXPORT extern "C" __declspec(dllexport)


typedef BOOL (WINAPI* CloseHandle_t)(HANDLE);
typedef int  (WINAPI* MessageBoxA_t)(HWND hwnd, LPCSTR text, LPCSTR caption, UINT type);
static CloseHandle_t TrampCloseHandle = NULL;
static MessageBoxA_t TrampMessageBox  = NULL;

BOOL WINAPI hookedCloseHandle(HANDLE handle);
int  WINAPI hookedMessageBoxA(HWND hwnd, LPCSTR text, LPCSTR caption, UINT type);

EXPORT int hook(const char* functionList) {
    TrampCloseHandle = CloseHandle;
    TrampMessageBox  = MessageBoxA;

    funchook_t* handle = funchook_create();

    if (funchook_prepare(handle, (void**)&TrampCloseHandle, hookedCloseHandle) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(handle);
        return 1;
    }
    if (funchook_prepare(handle, (void**)&TrampMessageBox,  hookedMessageBoxA) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(handle);
        return 1;
    }

    if (funchook_install(handle, 0) != FUNCHOOK_ERROR_SUCCESS) {
        funchook_destroy(handle);
        return 1;
    }

    return 0;
}

typedef NTSTATUS (NTAPI* NtQueryObject_t)(HANDLE, UINT, LPVOID, size_t, size_t*);
BOOL WINAPI hookedCloseHandle(HANDLE handle) {
    char flags[2];
    HMODULE ntdll = LoadLibraryA("ntdll.dll");
    NtQueryObject_t nqo = (NtQueryObject_t)GetProcAddress(ntdll, "NtQueryObject");
    NTSTATUS Status = nqo(handle, 4, &flags, 2, nullptr);

    if (Status >= 0)
    {
        if (flags[0])
            return false;

        return TrampCloseHandle(handle);
    }

    return false;
}
int  WINAPI hookedMessageBoxA(HWND hwnd, LPCSTR text, LPCSTR caption, UINT type) {
    printf("MessageBoxA(0x%x, \"%s\", \"%s\", 0x%x)\n", hwnd, text, caption, type);
    return TrampMessageBox(hwnd, text, caption, type);
}
