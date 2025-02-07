#include <Windows.h>
#include "ProcessMem.cpp"
#include <filesystem>
#include "VerDef.cpp"

ProcessMem PM;
VerDef VD;

using namespace std;

string FullPath = PM.GetProcessPathByPID(GetCurrentProcessId());
filesystem::path p(FullPath);
string filename = p.filename().string();
uintptr_t moduleBase = PM.GetModuleBaseAddress(GetCurrentProcessId(), filename.c_str());
HANDLE hProcess = GetCurrentProcess();

filesystem::path ini_file_path = filesystem::current_path() / "CD_CarLightsOnOff.ini";

int game_state;
int pause_state;
int car_lights_state;

float game_speed;

DWORD WINAPI MainTHREAD(LPVOID)
{
    int def_keyactivation = GetPrivateProfileIntA("CD_CarLightsOnOff", "KeyActivation", 76, ini_file_path.string().c_str());
    int gamever = VD.Init(hProcess, moduleBase);

    if (gamever == VERSION_12)
    {
        while (true)
        {
            if (PM.IsCurrentProcessActive())
            {
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DDEE0, { 0x14, 0xA0, 0x104, 0x4, 0x20, 0x4, 0x10 })), &game_state, 4, 0);
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DEA20, { 0x10, 0x4, 0x734 })), &pause_state, 1, 0);
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DC550, { 0x8 })), &game_speed, 4, 0);
                if ((game_state == 1) && (pause_state == 0) && (game_speed != 0.0f))
                {
                    if (GetAsyncKeyState(def_keyactivation) & 1)
                    {
                        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 0x4, 0x4, 0x72F8 })), &car_lights_state, 1, 0);
                        if (car_lights_state == 0)
                        {
                            car_lights_state = 1;
                        }
                        else
                        {
                            car_lights_state = 0;
                        }
                        WriteProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 0x4, 0x4, 0x72F8 })), &car_lights_state, 1, 0);
                    }
                }
            }
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, MainTHREAD, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

