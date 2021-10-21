#include <Windows.h>


int Check()
{
    __try
    {
        CloseHandle((HANDLE)0xDEADBEEF);
        return 0;
    }
    __except (EXCEPTION_INVALID_HANDLE == GetExceptionCode()
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH)
    {
        return 1;
    }
}

int main() {
    Sleep(10000);

    if (Check()) {
        MessageBoxA(0, "Detect debugger", NULL, 0);
        return 1;
    } else {
        MessageBoxA(0, "PASS NtClose", NULL, 0);
        return 0;
    }
}
