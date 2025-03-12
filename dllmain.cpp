#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <format>
#include <filesystem>
#include <unordered_map>
#include <windows.h>

#include "inipp.h"

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
std::vector<std::unique_ptr<std::string>> AdditionalDLLs;


std::unordered_map<std::string, BOOL*> settingsMap = {
    {"ShowMessageBox", &ShowMessageBox},
    {"LoadAdditionalDLLs", &LoadAdditionalDLLs},
    {"CloseProcessOnDLLLoadFailure", &CloseProcessOnDLLLoadFailure},
    {"PrintStatusOnLoad", &PrintStatusOnLoad}
};

void LoadINI()
{
    std::filesystem::path path = std::filesystem::current_path() / "dinput8proxy.ini";
    std::cout << "Loading INI file: " << path << std::endl;

    std::ifstream iniFile(path.string());
    
    if(iniFile.fail())
    {
        // Create a default INI file
        std::ofstream file(path.string());
        file << "[Settings]\n";
        for (const auto& setting : settingsMap)
        {
            file << setting.first << " = " << *setting.second << "\n";
        }
        file.close();
        iniFile.open(path.string());
    }

    inipp::Ini<char> ini;
    ini.parse(iniFile);
    ini.interpolate();

    for (const auto& setting : settingsMap)
    {
        std::string value;
        if (inipp::get_value(ini.sections["Settings"], setting.first, value))
        {
            *setting.second = std::stoi(value) != 0;
        }
    }

    iniFile.close();
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

    std::unique_ptr<std::string> contents = std::make_unique<std::string>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::istringstream stream(*contents);

    // Cleanup contents of empty lines, and remove comments
    for (std::string line; std::getline(stream, line);)
    {
        if (line.empty() || line[0] == ';')
        {
            continue;
        }
        AdditionalDLLs.push_back(std::make_unique<std::string>(line));
    }

    file.close();    
}

void PrintStatus()
{
    std::cout << "DINPUT8 Proxy Console" << std::endl;
    std::cout << "ShowMessageBox: " << ShowMessageBox << std::endl;
    std::cout << "LoadAdditionalDLLs: " << LoadAdditionalDLLs << std::endl;
    std::cout << "CloseProcessOnDLLLoadFailure: " << CloseProcessOnDLLLoadFailure << std::endl;
    std::cout << "AdditionalDLLs: " << AdditionalDLLs.size() << std::endl;
    for (const auto& dll : AdditionalDLLs)
    {
        std::cout << "  " << dll << std::endl;
    }
    std::cout << std::endl;
}

// Load additional DLLs from the AdditionalDLLs array
void DoLoadAdditionalDLLs()
{
    if (!LoadAdditionalDLLs) return;

    DoLoadAdditionalDLLsFile();

    for (size_t i = 0; i < AdditionalDLLs.size(); i++)
    {
		HMODULE hModule = LoadLibraryA(AdditionalDLLs[i]->c_str());
        if (!hModule)
        {
			std::cout << "Failed to load additional DLL: " << AdditionalDLLs[i]->c_str() << std::endl;
            
            if (CloseProcessOnDLLLoadFailure)
            {
                Sleep(5000);
                ExitProcess(1);
            }
        }
        else
        {
            std::cout << "Loaded additional DLL: " << AdditionalDLLs[i]->c_str() << std::endl;
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
    // SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
}


void DoStuff()
{
    LoadOriginalDLL();

    AllocateConsole();

    LoadINI();

    if (ShowMessageBox)
    {
        MessageBoxA(0, "DINPUT8 Proxy Loaded", "Success", MB_ICONINFORMATION);
    }


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

