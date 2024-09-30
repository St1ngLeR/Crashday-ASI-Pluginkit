#include "pch.h"
#include "ProcessMem.cpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

ProcessMem PM;

string FullPath = PM.GetProcessPathByPID(GetCurrentProcessId());
filesystem::path p(FullPath);
string filename = p.filename().string();
uintptr_t moduleBase = PM.GetModuleBaseAddress(GetCurrentProcessId(), filename.c_str());
HANDLE hProcess = GetCurrentProcess();

void Main()
{
    char aftbur_path[14];
    char aftbur_filename[11];

    float aftbur_acceleration;
    float aftbur_heatrisespeed;
    float aftbur_heatcooldownspeed;

    ReadProcessMemory(hProcess, (void*)(moduleBase + 0x2B423E), &aftbur_path, sizeof(aftbur_path), 0);
    ReadProcessMemory(hProcess, (void*)(moduleBase + 0x2B424C), &aftbur_filename, sizeof(aftbur_filename), 0);

    filesystem::path aftbur_fullpath(p.parent_path() / aftbur_path / aftbur_filename);

    std::ifstream file(aftbur_fullpath);
    std::string line;
    int lineCounter = 0;
    int lineIndex;
    int foundNext = 0;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            lineCounter++;

            if (line.find("Next:") != std::string::npos) {
                foundNext++;
                lineIndex = lineCounter;
            }
            if (foundNext == 1)
            {
                if (lineCounter == lineIndex + 5)
                {
                    aftbur_acceleration = stof(line.substr(0, line.find("#")));
                    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x6AA36), &aftbur_acceleration, 4, 0);
                }
                if (lineCounter == lineIndex + 6)
                {
                    aftbur_heatrisespeed = stof(line.substr(0, line.find("#")));
                    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x6AA5E), &aftbur_heatrisespeed, 4, 0);
                }
                if (lineCounter == lineIndex + 7)
                {
                    aftbur_heatcooldownspeed = stof(line.substr(0, line.find("#")));
                    WriteProcessMemory(hProcess, (void*)(moduleBase + 0x6AA88), &aftbur_heatcooldownspeed, 4, 0);
                }
            }
        }
        file.close();
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
        Main();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

