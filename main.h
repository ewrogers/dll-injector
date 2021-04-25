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

#endif //INJECTOR_MAIN_H
