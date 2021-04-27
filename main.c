#include "main.h"

const LPCSTR DA_WINDOW_CLASS_NAME = "DarkAges";
const LPCSTR CONSOLE_COMMAND = "console";
const LPCSTR CHECK_COMMAND = "check";
const LPCSTR INJECT_COMMAND = "inject";
const LPCSTR UNLOAD_COMMAND = "unload";
const LPCSTR SHOW_HELP_COMMAND = "help";

int main(int argc, char *argv[]) {
    // Check enough arguments were passed
    if (argc < 2) {
        printf("Invalid arguments\n");
        showHelp();
        return 1;
    }

    char *command = argv[1];
    char *parameter = argv[2];

    // Help was requested, show and exit
    if (!_strcmpi(command, SHOW_HELP_COMMAND)) {
        showHelp();
        return 0;
    }

    // CONSOLE command
    if (!_strcmpi(command, CONSOLE_COMMAND)) {
        printf("Finding DarkAges game client...\n");
        DWORD processId = getProcessByWindow(DA_WINDOW_CLASS_NAME, NULL);
        if (!processId) {
            printf("No game clients were found!\n");
            return 2;
        }

        HANDLE hProcess = openRemoteProcess(processId);
        if (!hProcess) {
            printf("Unable to open process!\n");
            return 2;
        }

        printf("Allocating remote console...\n");
        if (!allocConsoleRemote(hProcess)) {
            printf("Unable to allocate console window!\n");
            CloseHandle(hProcess);
            return 2;
        }

        CloseHandle(hProcess);
        return 0;
    }

    // CHECK command
    if (!_strcmpi(command, CHECK_COMMAND)) {
        if (!parameter || strlen(parameter) == 0) {
            printf("Missing DLL name\n");
            showHelp();
            return 1;
        }

        printf("Finding DarkAges game client...\n");
        DWORD processId = getProcessByWindow(DA_WINDOW_CLASS_NAME, NULL);
        if (!processId) {
            printf("No game clients were found!\n");
            return 2;
        }

        HANDLE hProcess = openRemoteProcess(processId);
        if (!hProcess) {
            printf("Unable to open process!\n");
            return 2;
        }
        printf("Checking if library loaded into game client... (pid=%d)\n", (int)processId);

        char filename[MAX_PATH];
        char extension[MAX_PATH];
        _splitpath_s(parameter, NULL, 0, NULL, 0, filename, sizeof(filename), extension, sizeof(extension));
        strcat_s(filename, sizeof(filename), extension);

        HMODULE hModule = getModuleHandleRemote(hProcess, filename);
        if (hModule) {
            printf("Library was found at 0x%p\n", hModule);
        } else {
            printf("Library not loaded\n");
        }

        CloseHandle(hProcess);
        return 0;
    }

    // INJECT command
    if (!_strcmpi(command, INJECT_COMMAND)) {
        if (!parameter || strlen(parameter) == 0) {
            printf("Missing DLL path\n");
            showHelp();
            return 1;
        }

        printf("Finding DarkAges game client...\n");
        DWORD processId = getProcessByWindow(DA_WINDOW_CLASS_NAME, NULL);
        if (!processId) {
            printf("No game clients were found!\n");
            return 2;
        }

        HANDLE hProcess = openRemoteProcess(processId);
        if (!hProcess) {
            printf("Unable to open process!\n");
            return 2;
        }
        printf("Injecting library into game client... (pid=%d)\n", (int)processId);

        char filename[MAX_PATH];
        char extension[MAX_PATH];
        _splitpath_s(parameter, NULL, 0, NULL, 0, filename, sizeof(filename), extension, sizeof(extension));
        strcat_s(filename, sizeof(filename), extension);

        HMODULE hModule = getModuleHandleRemote(hProcess, filename);
        if (hModule) {
            printf("Already loaded, unloading library...\n");
        }
        while (hModule) {
            freeLibraryRemote(hProcess, hModule);
            hModule = getModuleHandleRemote(hProcess, filename);
        }

        hModule = loadLibraryRemote(hProcess, parameter);
        if (hModule) {
            printf("Successfully injected library at 0x%p\n", hModule);
        } else {
            printf("Unable to inject library!\n");
        }

        CloseHandle(hProcess);
        return hModule != 0 ? 0 : 2;
    }

    // UNLOAD command
    if (!_strcmpi(command, UNLOAD_COMMAND)) {
        if (!parameter || strlen(parameter) == 0) {
            printf("Missing DLL name\n");
            showHelp();
            return 1;
        }

        printf("Finding DarkAges game client...\n");
        DWORD processId = getProcessByWindow(DA_WINDOW_CLASS_NAME, NULL);
        if (!processId) {
            printf("No game clients were found!\n");
            return 2;
        }

        HANDLE hProcess = openRemoteProcess(processId);
        if (!hProcess) {
            printf("Unable to open process!\n");
            return 2;
        }
        printf("Checking if library loaded into game client... (pid=%d)\n", (int)processId);

        char filename[MAX_PATH];
        char extension[MAX_PATH];
        _splitpath_s(parameter, NULL, 0, NULL, 0, filename, sizeof(filename), extension, sizeof(extension));
        strcat_s(filename, sizeof(filename), extension);

        HMODULE hModule = getModuleHandleRemote(hProcess, filename);
        if (hModule) {
            printf("Library was found at 0x%p, unloading library...\n", hModule);
        } else {
            printf("Library not loaded, nothing to do\n");
        }
        while (hModule) {
            freeLibraryRemote(hProcess, hModule);
            hModule = getModuleHandleRemote(hProcess, filename);
        }

        CloseHandle(hProcess);
        return 0;
    }
}

void showHelp() {
    printf("Available commands:\n");
    printf("console - allocates and attaches to the console of a remote process\n");
    printf("  usage: injector console\n");
    printf("check - checks if a DLL has been loaded into a remote process\n");
    printf("  usage: injector check <dll_name>\n");
    printf("inject - injects a DLL into a remote process\n");
    printf("  usage: injector inject <dll_path>\n");
    printf("unload - unloads a DLL from a remote process\n");
    printf("  usage: injector unload <dll_name>\n");
    printf("help - shows this help information\n");
}

DWORD getProcessByWindow(LPCSTR windowClass, LPCSTR windowTitle) {
    DWORD processId = 0;
    HWND hWnd = FindWindowA(windowClass, windowTitle);
    if (hWnd) {
        GetWindowThreadProcessId(hWnd, &processId);
    }
    return processId;
}

HANDLE openRemoteProcess(DWORD processId) {
    return OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
                FALSE,
                processId);
}

HMODULE getModuleHandleRemote(HANDLE hProcess, LPCSTR moduleName) {
    DWORD exitCode;
    return remoteCallAllocString(hProcess, "kernel32.dll", "GetModuleHandleA", moduleName, &exitCode)
        ? (HMODULE)exitCode
        : NULL;
}

HMODULE loadLibraryRemote(HANDLE hProcess, LPCSTR libraryPath) {
    DWORD exitCode;
    return remoteCallAllocString(hProcess, "kernel32.dll", "LoadLibraryA", libraryPath, &exitCode)
        ? (HMODULE)exitCode
        : NULL;
}

BOOL freeLibraryRemote(HANDLE hProcess, HMODULE hModule) {
    DWORD exitCode;
    return remoteCallNonAlloc(hProcess, "kernel32.dll", "FreeLibrary", hModule, &exitCode)
        ? exitCode != 0
        : FALSE;
}

BOOL allocConsoleRemote(HANDLE hProcess) {
    DWORD exitCode;
    return remoteCallNonAlloc(hProcess, "kernel32.dll", "AllocConsole", NULL, &exitCode);
}

BOOL attachConsoleRemote(HANDLE hProcess, DWORD processId) {
    DWORD exitCode;
    return remoteCallNonAlloc(hProcess, "kernel32.dll", "AttachConsole", (LPVOID)processId, &exitCode);
}

BOOL freeConsoleRemote(HANDLE hProcess) {
    DWORD exitCode;
    return remoteCallNonAlloc(hProcess, "kernel32.dll", "AttachConsole", NULL, &exitCode);
}

BOOL remoteCallAllocString(HANDLE hProcess, LPCSTR moduleName, LPCSTR procName, LPCSTR stringValue, LPDWORD exitCode) {
    DWORD memSize = strlen(stringValue) + 1;
    LPVOID memPtr = VirtualAllocEx(hProcess, 0, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!memPtr) return FALSE;

    DWORD bytesWritten;
    if (!WriteProcessMemory(hProcess, memPtr, stringValue, memSize, &bytesWritten)) {
        VirtualFreeEx(hProcess, memPtr, memSize, MEM_RELEASE);
        return FALSE;
    }

    char readBuffer[MAX_PATH];
    memset(readBuffer, 0, sizeof(readBuffer));

    DWORD bytesRead;
    ReadProcessMemory(hProcess, memPtr, readBuffer, memSize, &bytesRead);

    BOOL success = remoteCallNonAlloc(hProcess, moduleName, procName, memPtr, exitCode);

    VirtualFreeEx(hProcess, memPtr, memSize, MEM_RELEASE);
    return success;
}

BOOL remoteCallNonAlloc(HANDLE hProcess, LPCSTR moduleName, LPCSTR procName, LPVOID argument, LPDWORD exitCode) {
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (!hModule) return FALSE;

    FARPROC remoteProc = GetProcAddress(hModule, procName);
    if (!remoteProc) return FALSE;

    DWORD threadId;
    HANDLE hThread = CreateRemoteThread(hProcess,
                                        NULL,
                                        0,
                                        (LPTHREAD_START_ROUTINE)remoteProc,
                                        argument,
                                        0,
                                        &threadId);

    if (!hThread) return FALSE;

    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, exitCode);

    CloseHandle(hThread);
    return TRUE;
}
