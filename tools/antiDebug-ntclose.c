#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>


int Check()
{
    __try
    {
    printf("youke1\n");
        CloseHandle((HANDLE)0xDEADBEEF);
    printf("youke2\n");
        return 0;
    }
    __except (EXCEPTION_INVALID_HANDLE == GetExceptionCode()
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH)
    {
    printf("youke3\n");
        return 1;
    }
}

int main() {
    Sleep(1);
    Sleep(1);
    Sleep(1);
    Sleep(1);

    if (Check()) {
    printf("youke5\n");
        printf("Detect Debugger -- NtClose()\n");
        MessageBoxA(0, "Detect debugger", NULL, 0);
    printf("youke6\n");
        return 1;
    } else {
    printf("youke7\n");
        printf("PASS NtClose()\n");
        MessageBoxA(0, "PASS NtClose", NULL, 0);
    printf("youke8\n");
        return 0;
    }
}
