#include <cstdio>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <format>
#include <windows.h>

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

BOOL ShowMessageBox = true;
BOOL LoadAdditionalDLLs = true;
BOOL CloseProcessOnDLLLoadFailure = true;
BOOL PrintStatusOnLoad = true;

// Additional DLLs
LPSTR AdditionalDLLs[256];
int AdditionalDLLCount = 0;

// Load or create the default INI file
void LoadINI()
{
    char path[MAX_PATH];
    GetModuleFileNameA(0, path, MAX_PATH);
    strcat_s(path, ".ini");

    ShowMessageBox = GetPrivateProfileIntA("Settings", "ShowMessageBox", 1, path);
    LoadAdditionalDLLs = GetPrivateProfileIntA("Settings", "LoadAdditionalDLLs", 1, path);
    CloseProcessOnDLLLoadFailure = GetPrivateProfileIntA("Settings", "CloseProcessOnDLLLoadFailure", 1, path);
    PrintStatusOnLoad = GetPrivateProfileIntA("Settings", "PrintStatusOnLoad", 1, path);
}

void CreateDefaultAdditionalDLLsFile()
{
    std::ofstream file("AdditionalDLLs.txt");
    file << "Susano.dll\n";
    file.close();
}

void DoLoadAdditionalDLLsFile()
{
    if (!LoadAdditionalDLLs) return;

    std::ifstream file("AdditionalDLLs.txt");

    if (!file)
    {
        MessageBoxA(0, "Failed to open AdditionalDLLs.txt", "Error", MB_ICONERROR);
        CreateDefaultAdditionalDLLsFile();
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

        printf("Found additional DLL: %s\n", line.c_str());

        // Add the DLL to the list
        AdditionalDLLs[AdditionalDLLCount] = (LPSTR)line.c_str();
        AdditionalDLLCount++;
    }

    file.close();

    if (AdditionalDLLCount == 0)
    {
        MessageBoxA(0, "No additional DLLs found in AdditionalDLLs.txt", "Error", MB_ICONERROR);
        ExitProcess(1);
    }
}

void PrintStatus()
{
    printf("DINPUT8 Proxy Console\n");
    printf("ShowMessageBox: %d\n", ShowMessageBox);
    printf("LoadAdditionalDLLs: %d\n", LoadAdditionalDLLs);
    printf("CloseProcessOnDLLLoadFailure: %d\n", CloseProcessOnDLLLoadFailure);
    printf("AdditionalDLLs: %d\n", AdditionalDLLCount);
    for (int i = 0; i < AdditionalDLLCount; i++)
    {
        printf("  %s\n", AdditionalDLLs[i]);
    }
}

// Load additional DLLs from the AdditionalDLLs array
void DoLoadAdditionalDLLs()
{
    if (!LoadAdditionalDLLs) return;

    DoLoadAdditionalDLLsFile();

    for (int i = 0; i < AdditionalDLLCount; i++)
    {
        HMODULE hModule = LoadLibraryA(AdditionalDLLs[i]);
        if (!hModule)
        {
            MessageBoxA(0, std::format("Failed to load additional DLL: {0}", AdditionalDLLs[i]).c_str(), "Error", MB_ICONERROR);
            ExitProcess(1);
        }
        else
        {
            printf("Loaded additional DLL: %s\n", AdditionalDLLs[i]);
        }
    }
}
    
void AllocateConsole()
{
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);

    SetConsoleTitleA("DINPUT8 Proxy Console");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
}


void DoStuff()
{
    LoadOriginalDLL();

    LoadINI();

    if (ShowMessageBox)
    {
        MessageBoxA(0, "DINPUT8 Proxy Loaded", "Success", MB_ICONINFORMATION);
    }

    AllocateConsole();

    PrintStatus();

    DoLoadAdditionalDLLs();
}

// Proxy functions
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DoStuff();
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

