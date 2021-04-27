# dll-injector

## Overview

This command-line utility is used to load and unload DLLs into remote processes (DarkAges clients). It offers a few simple commands.

## Commands

### console
Allocates a console window for the remote process, allowing `printf` and other standard output to be viewable.

**Usage:**
```
injector console
```

### check
Checks if a DLL has been loaded into a remote process.

**Usage:**
```
injector check <dll_name>
```
Example: `injector check winsock.dll`

### inject
Injects a DLL into a remote process, unloading it first if already loaded. Absolute paths are recommended, since relative paths will be resolved in the context of the remote process' current working directory!

**Usage:**
```
injector inject <dll_path>
```
Example: `injector inject C:\path\to\custom.dll`

This will cause `DllMain` to be called with the  `DLL_PROCESS_ATTACH` reason.

### unload
Unloads a DLL from a remote process. You may pass a full path, but only the filename portion is used.

**Usage:**
```
injector unload <dll_name>
```
Example: `injector unload custom.dll`

This will cause `DllMain` to be called with the  `DLL_PROCESS_DETACH` reason.
