#include <math.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <tchar.h>
#include "ntinfo.h"
#include <Psapi.h>

using namespace std;

class ProcessMem
{
public:
    bool IsCurrentProcessActive() {
        // Получаем дескриптор активного окна
        HWND activeWindow = GetForegroundWindow();

        if (activeWindow == NULL) {
            // Нет активного окна (например, экран заблокирован)
            return false;
        }

        // Получаем ID процесса, которому принадлежит активное окно
        DWORD activeProcessId;
        GetWindowThreadProcessId(activeWindow, &activeProcessId);

        // Получаем ID текущего процесса
        DWORD currentProcessId = GetCurrentProcessId();

        // Сравниваем ID процессов
        return (activeProcessId == currentProcessId);
    }

    std::vector<DWORD> threadList(DWORD pid) {
        /* solution from http://stackoverflow.com/questions/1206878/enumerating-threads-in-windows */
        std::vector<DWORD> vect = std::vector<DWORD>();
        HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (h == INVALID_HANDLE_VALUE)
            return vect;

        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(h, &te)) {
            do {
                if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                    sizeof(te.th32OwnerProcessID)) {


                    if (te.th32OwnerProcessID == pid) {
                        printf("PID: %04d Thread ID: 0x%04x\n", te.th32OwnerProcessID, te.th32ThreadID);
                        vect.push_back(te.th32ThreadID);
                    }

                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(h, &te));
        }

        return vect;
    }

    DWORD GetThreadStartAddress(HANDLE processHandle, HANDLE hThread) {
        /* rewritten from https://github.com/cheat-engine/cheat-engine/blob/master/Cheat%20Engine/CEFuncProc.pas#L3080 */
        DWORD used = 0, ret = 0;
        DWORD stacktop = 0, result = 0;

        MODULEINFO mi;

        GetModuleInformation(processHandle, GetModuleHandle("kernel32.dll"), &mi, sizeof(mi));
        stacktop = (DWORD)GetThreadStackTopAddress_x86(processHandle, hThread);

        /* The stub below has the same result as calling GetThreadStackTopAddress_x86()
        change line 54 in ntinfo.cpp to return tbi.TebBaseAddress
        Then use this stub
        */
        //LPCVOID tebBaseAddress = GetThreadStackTopAddress_x86(processHandle, hThread);
        //if (tebBaseAddress)
        //	ReadProcessMemory(processHandle, (LPCVOID)((DWORD)tebBaseAddress + 4), &stacktop, 4, NULL);

        /* rewritten from 32 bit stub (line3141)
        Result: fail -- can't get GetThreadContext()
        */
        //CONTEXT context;
        //LDT_ENTRY ldtentry;
        //GetModuleInformation(processHandle, LoadLibrary("kernel32.dll"), &mi, sizeof(mi));
        //
        //if (GetThreadContext(processHandle, &context)) {
        //	
        //	if (GetThreadSelectorEntry(hThread, context.SegFs, &ldtentry)) {
        //		ReadProcessMemory(processHandle,
        //			(LPCVOID)( (DWORD*)(ldtentry.BaseLow + ldtentry.HighWord.Bytes.BaseMid << ldtentry.HighWord.Bytes.BaseHi << 24) + 4),
        //			&stacktop,
        //			4,
        //			NULL);
        //	}
        //}

        CloseHandle(hThread);

        if (stacktop) {
            //find the stack entry pointing to the function that calls "ExitXXXXXThread"
            //Fun thing to note: It's the first entry that points to a address in kernel32

            DWORD* buf32 = new DWORD[4096];

            if (ReadProcessMemory(processHandle, (LPCVOID)(stacktop - 4096), buf32, 4096, NULL)) {
                for (int i = 4096 / 4 - 1; i >= 0; --i) {
                    if (buf32[i] >= (DWORD)mi.lpBaseOfDll && buf32[i] <= (DWORD)mi.lpBaseOfDll + mi.SizeOfImage) {
                        result = stacktop - 4096 + i * 4;
                        break;
                    }

                }
            }

            delete buf32;
        }

        return result;
    }

    string GetProcessPathByPID(DWORD processId) {
        // Получаем хэндл процесса
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess == NULL) {
            return ""; // Возвращаем пустую строку в случае ошибки
        }

        // Выделяем память для хранения пути
        TCHAR path[MAX_PATH] = { 0 };
        DWORD pathLength = MAX_PATH;

        // Получаем путь к исполняемому файлу процесса
        if (QueryFullProcessImageName(hProcess, 0, path, &pathLength)) {
            // Преобразуем путь в строку
            string processPath = path;
            return processPath;
        }
        else {
            CloseHandle(hProcess);
            return ""; // Возвращаем пустую строку в случае ошибки
        }
    }

    DWORD GetProcId(const char* procName)
    {
        DWORD procId = 0;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32 procEntry;
            procEntry.dwSize = sizeof(procEntry);

            if (Process32First(hSnap, &procEntry))
            {
                do
                {
                    if (!_stricmp(procEntry.szExeFile, procName))
                    {
                        procId = procEntry.th32ProcessID;
                        break;
                    }
                } while (Process32Next(hSnap, &procEntry));

            }
        }
        CloseHandle(hSnap);
        return procId;
    }

    uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName)
    {
        uintptr_t modBaseAddr = 0;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
        if (hSnap != INVALID_HANDLE_VALUE)
        {
            MODULEENTRY32 modEntry;
            modEntry.dwSize = sizeof(modEntry);
            if (Module32First(hSnap, &modEntry))
            {
                do
                {
                    if (!_stricmp(modEntry.szModule, modName))
                    {
                        modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                        break;
                    }
                } while (Module32Next(hSnap, &modEntry));
            }
        }
        CloseHandle(hSnap);
        return modBaseAddr;
    }

    bool CompareFloat(float x, float y, float epsilon = 0.01f) {
        if (fabs(x - y) < epsilon)
            return true; //they are same
        return false; //they are not same
    }

    uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
    {
        uintptr_t addr = ptr;
        for (unsigned int i = 0; i < offsets.size(); ++i)
        {
            ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
            addr += offsets[i];
        }
        return addr;
    }

};