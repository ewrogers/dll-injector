#include "main.h"

const LPCSTR DA_WINDOW_CLASS_NAME = "DarkAges";
const LPCSTR LOAD_COMMAND = "load";
const LPCSTR CHECK_COMMAND = "check";
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

    // CHECK command
    if (!_strcmpi(command, "check")) {
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

    // LOAD command
    if (!_strcmpi(command, "load")) {
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
    if (!_strcmpi(command, "unload")) {
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
    printf("check - checks if a DLL has been loaded into a remote process\n");
    printf("  usage: injector check <dll_name>\n");
    printf("load - injects a DLL into a remote process\n");
    printf("  usage: injector load <dll_path>\n");
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
    DWORD remoteModule = 0;
    FARPROC getModuleHandleProc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetModuleHandleA");

    DWORD memSize = strlen(moduleName) + 1;
    LPVOID memPtr = VirtualAllocEx(hProcess, 0, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HANDLE hThread = NULL;

    if (!memPtr) goto cleanup;

    DWORD bytesWritten;
    if (!WriteProcessMemory(hProcess, memPtr, moduleName, memSize, &bytesWritten)) {
        goto cleanup;
    }

    DWORD threadId;
    hThread = CreateRemoteThread(hProcess,
                                 NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)getModuleHandleProc,
                                 memPtr,
                                 0,
                                 &threadId);

    if (!hThread) goto cleanup;

    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &remoteModule);

    cleanup:
    memPtr && VirtualFreeEx(hProcess, memPtr, memSize, MEM_RELEASE);
    hThread && CloseHandle(hThread);
    return (HMODULE)remoteModule;
}

HMODULE loadLibraryRemote(HANDLE hProcess, LPCSTR libraryPath) {
    DWORD remoteModule = 0;
    FARPROC loadLibraryProc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    DWORD memSize = strlen(libraryPath) + 1;
    LPVOID memPtr = VirtualAllocEx(hProcess, 0, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HANDLE hThread = NULL;

    if (!memPtr) goto cleanup;

    DWORD bytesWritten;
    if (!WriteProcessMemory(hProcess, memPtr, libraryPath, memSize, &bytesWritten)) {
        goto cleanup;
    }

    DWORD threadId;
    hThread = CreateRemoteThread(hProcess,
                                 NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)loadLibraryProc,
                                 memPtr,
                                 0,
                                 &threadId);

    if (!hThread) goto cleanup;

    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &remoteModule);

    cleanup:
    memPtr && VirtualFreeEx(hProcess, memPtr, memSize, MEM_RELEASE);
    hThread && CloseHandle(hThread);
    return (HMODULE)remoteModule;
}

BOOL freeLibraryRemote(HANDLE hProcess, HMODULE hModule) {
    FARPROC freeLibraryProc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");

    DWORD threadId;
    HANDLE hThread = CreateRemoteThread(hProcess,
                                 NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)freeLibraryProc,
                                 hModule,
                                 0,
                                 &threadId);

    if (!hThread) return FALSE;

    DWORD exitCode;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    return exitCode != 0;
}
