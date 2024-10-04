#include "pch.h"
#include "ProcessMem.cpp"
#include <filesystem>

using namespace std;

ProcessMem PM;

string FullPath = PM.GetProcessPathByPID(GetCurrentProcessId());
filesystem::path p(FullPath);
string filename = p.filename().string();
uintptr_t moduleBase = PM.GetModuleBaseAddress(GetCurrentProcessId(), filename.c_str());
HANDLE hProcess = GetCurrentProcess();

void Main()
{
    int val1 = 0x1D;
    int val2 = 0x00;
    float percent_step = 1.f / 29.f;

    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x112207), &val1, 1, 0);
    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x2C927E), &percent_step, 4, 0);
    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x114847), &val2, 1, 0);
    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x114A22), &val2, 1, 0);
    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x1188EF), &val2, 1, 0);
    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x114B97), &val2, 1, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Main();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

