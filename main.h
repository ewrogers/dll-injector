#ifndef INJECTOR_MAIN_H
#define INJECTOR_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc, char *argv[]);
void showHelp();

DWORD getProcessByWindow(LPCSTR windowClass, LPCSTR windowTitle);

HANDLE openRemoteProcess(DWORD processId);
HMODULE getModuleHandleRemote(HANDLE hProcess, LPCSTR moduleName);
HMODULE loadLibraryRemote(HANDLE hProcess, LPCSTR libraryPath);
BOOL freeLibraryRemote(HANDLE hProcess, HMODULE hModule);

BOOL allocConsoleRemote(HANDLE hProcess);

BOOL remoteCallAllocString(HANDLE hProcess, LPCSTR moduleName, LPCSTR procName, LPCSTR stringValue, LPDWORD exitCode);
BOOL remoteCallNonAlloc(HANDLE hProcess, LPCSTR moduleName, LPCSTR procName, LPVOID value, LPDWORD exitCode);

#endif //INJECTOR_MAIN_H
