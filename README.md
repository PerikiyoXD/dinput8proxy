# dinput8proxy

Painless, universal DInput8 proxy DLL for games and mods. 
Allows loading additional DLL files to extend functionality.
Call it, the universal mod loader.

# Features
- Load additional DLL files defined in `dinput8proxy.ini`.

# Configuration
The `dinput8proxy.ini` file is used to configure the proxy DLL. The following settings are available:

- `ShowMessageBox`: Show a message box when the proxy DLL is loaded (default: 1).
- `LoadAdditionalDLLs`: Load additional DLLs specified in `AdditionalDLLs.txt` (default: 1).
- `CloseProcessOnDLLLoadFailure`: Close the process if an additional DLL fails to load (default: 1).
- `PrintStatusOnLoad`: Print the status to the console when the proxy DLL is loaded (default: 1).

# Loading Additional DLLs
The `AdditionalDLLs.txt` file is used to specify additional DLLs to be loaded. 
Line order is respected, and the DLLs are loaded in the order they appear in the file.
Each line in the file should contain the name of a DLL to be loaded. For example:

```
; this is a comment line, so it will be ignored
; the following DLLs will be loaded
Susano.dll
AnotherDLL.dll
; this one not, as it's commented out
; NotThisOne.dll
```