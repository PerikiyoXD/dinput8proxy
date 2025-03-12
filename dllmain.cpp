#include <windows.h>
#include <iostream>

// Forward declare DINPUT8 functions
typedef HRESULT(WINAPI* DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(WINAPI* DllCanUnloadNow_t)();
typedef HRESULT(WINAPI* DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT(WINAPI* DllRegisterServer_t)();
typedef HRESULT(WINAPI* DllUnregisterServer_t)();
typedef HRESULT(WINAPI* GetdfDIJoystick_t)();

HMODULE hOriginalDLL = NULL;
DirectInput8Create_t orig_DirectInput8Create = nullptr;
DllCanUnloadNow_t orig_DllCanUnloadNow = nullptr;
DllGetClassObject_t orig_DllGetClassObject = nullptr;
DllRegisterServer_t orig_DllRegisterServer = nullptr;
DllUnregisterServer_t orig_DllUnregisterServer = nullptr;
GetdfDIJoystick_t orig_GetdfDIJoystick = nullptr;

// Load the original DINPUT8.dll
void LoadOriginalDLL()
{
    if (!hOriginalDLL)
    {
        char systemPath[MAX_PATH];
        GetSystemDirectoryA(systemPath, MAX_PATH);
        strcat_s(systemPath, "\\DINPUT8.dll");
        hOriginalDLL = LoadLibraryA(systemPath);

        if (!hOriginalDLL)
        {
            MessageBoxA(0, "Failed to load original DINPUT8.dll", "Error", MB_ICONERROR);
            ExitProcess(1);
        }

        // Load function pointers
        orig_DirectInput8Create = (DirectInput8Create_t)GetProcAddress(hOriginalDLL, "DirectInput8Create");
        orig_DllCanUnloadNow = (DllCanUnloadNow_t)GetProcAddress(hOriginalDLL, "DllCanUnloadNow");
        orig_DllGetClassObject = (DllGetClassObject_t)GetProcAddress(hOriginalDLL, "DllGetClassObject");
        orig_DllRegisterServer = (DllRegisterServer_t)GetProcAddress(hOriginalDLL, "DllRegisterServer");
        orig_DllUnregisterServer = (DllUnregisterServer_t)GetProcAddress(hOriginalDLL, "DllUnregisterServer");
        orig_GetdfDIJoystick = (GetdfDIJoystick_t)GetProcAddress(hOriginalDLL, "GetdfDIJoystick");
    }
}

// Proxy functions
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        MessageBoxA(0, "Hello from DINPUT8.dll", "Info", MB_ICONINFORMATION);
        LoadOriginalDLL();
    }
    return TRUE;
}

// Export functions
extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    return orig_DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

extern "C" HRESULT WINAPI DllCanUnloadNow()
{
    return orig_DllCanUnloadNow();
}

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return orig_DllGetClassObject(rclsid, riid, ppv);
}

extern "C" HRESULT WINAPI DllRegisterServer()
{
    return orig_DllRegisterServer();
}

extern "C" HRESULT WINAPI DllUnregisterServer()
{
    return orig_DllUnregisterServer();
}

extern "C" HRESULT WINAPI GetdfDIJoystick()
{
    return orig_GetdfDIJoystick();
}

// Export function directives
#pragma comment(linker, "/export:DirectInput8Create=DirectInput8Create,@1")
#pragma comment(linker, "/export:DllCanUnloadNow=DllCanUnloadNow,PRIVATE")
#pragma comment(linker, "/export:DllGetClassObject=DllGetClassObject,PRIVATE")
#pragma comment(linker, "/export:DllRegisterServer=DllRegisterServer,PRIVATE")
#pragma comment(linker, "/export:DllUnregisterServer=DllUnregisterServer,PRIVATE")
#pragma comment(linker, "/export:GetdfDIJoystick=GetdfDIJoystick,@6")

