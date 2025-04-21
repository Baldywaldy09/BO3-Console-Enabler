#include "Windows.h"
#include <iostream>
#include <chrono>
#include <iomanip>
bool shouldUnload = false;

#include <MinHook/MinHook.h>
#pragma comment(lib, "minhook.x64.lib")

uint32_t get_time_ms()
{
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return static_cast<uint32_t>(duration_cast<milliseconds>(steady_clock::now() - start).count());
}

typedef char(__fastcall* original_func_t) (int a1, unsigned int a2, const char* a3, int a4);
original_func_t original_func;

void redirected_logger(int a1, unsigned int a2, const char* a3, int a4)
{
    if (a3)
        printf("[BlackOps3] %s", a3);

    original_func(a1, a2, a3, a4);
}

FILE* fStream;
DWORD WINAPI MainThread(HMODULE hModule)
{
    AllocConsole();
    freopen_s(&fStream, "CONOUT$", "w", stdout);
    SetConsoleTitleA("Call of Duty® | Debug Console");

    printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    printf("[BO3-Console Enabler] BO3 Console Enabler | By: Baldy09 (github.com/Baldywaldy09)\n");
    printf("[BO3-Console Enabler] Plugin Loading...\n");
    

    uintptr_t gameBase = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    printf("[BO3-Console-Enabler] Game Base: %p\n", gameBase);


    uintptr_t loggerFunctionAddress;

    if (GetModuleHandleA("gamechat2.dll"))
    {
        printf("[BO3-Console-Enabler] Game type: UWP\n");

        uintptr_t offset = 0x220F4F6;
        loggerFunctionAddress = (gameBase + offset) - 6;
    }
    else
    {
        printf("[BO3-Console-Enabler] Game type: Steam\n");

        uintptr_t offset = 0x2148C06;
        loggerFunctionAddress = (gameBase + offset) - 6;
    }

    printf("[BO3-Console-Enabler] Found logger function at %p\n", loggerFunctionAddress);
    
    MH_Initialize();

    MH_CreateHook((LPVOID)loggerFunctionAddress, (LPVOID)redirected_logger, (LPVOID*)&original_func);
    MH_EnableHook((LPVOID)loggerFunctionAddress);

    printf("[BO3-Console-Enabler] Function hooked\n");
    printf("[BO3-Console-Enabler] Plugin Loaded!\n");
    printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n");

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        fclose(fStream);
        FreeConsole();
    }

    return true;
}