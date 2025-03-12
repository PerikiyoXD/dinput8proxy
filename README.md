# dinput8proxy

**A painless, universal DirectInput8 (DInput8) proxy DLL for extending functionality.**

This proxy DLL allows you to load additional DLL files effortlessly, making it a universal mod loader.

## Features
- Configurable proxy DLL via `dinput8proxy.ini`
- Supports loading additional DLLs specified in `AdditionalDLLs.txt`
- Customizable behavior through configuration options

### Configurable Settings:
- **Show a message box** when the proxy DLL is loaded
- **Terminate the process** if an additional DLL fails to load
- **Print status messages** to the console on load

## Configuration
The `dinput8proxy.ini` file allows you to configure the proxy DLL behavior. Available settings:

| Setting                      | Description                                                   | Default |
|------------------------------|---------------------------------------------------------------|---------|
| `ShowMessageBox`             | Displays a message box when the proxy DLL loads.            | `1`     |
| `LoadAdditionalDLLs`         | Loads extra DLLs from `AdditionalDLLs.txt`.                 | `1`     |
| `CloseProcessOnDLLLoadFailure` | Terminates the process if a DLL fails to load.            | `1`     |
| `PrintStatusOnLoad`          | Outputs a status message to the console when loaded.        | `1`     |

## Loading Additional DLLs
To load extra DLLs, list them in `AdditionalDLLs.txt`. The order matters, and they are loaded sequentially. Each line should contain the name of a DLL.

### Example `AdditionalDLLs.txt`:
```
; This is a comment line and will be ignored
; The following DLLs will be loaded:
Susano.dll
AnotherDLL.dll

; This one will NOT be loaded since it's commented out
; NotThisOne.dll
```

## Usage
1. Place `dinput8proxy.dll` in the application directory.
2. Configure settings in `dinput8proxy.ini` (optional).
3. Specify additional DLLs in `AdditionalDLLs.txt` (if needed).
4. Launch the application, and the proxy DLL will handle the rest.

---
This tool simplifies extending game and software functionality by seamlessly injecting additional DLLs. Happy modding!