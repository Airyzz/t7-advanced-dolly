// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "StdInc.h"
#include "Memory.h"
#include "Addresses.h"
#include "Hooks.h"
#include <string.h>
#include <cstring>

HMODULE hModule;
int procId;

using namespace Memory;
using namespace Addresses;
DWORD WINAPI Init(LPVOID Param) {

    ReadAllAddresses();
    Hooks::Initialise();

    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule_, DWORD  ul_reason_for_call, LPVOID lpReserved)
{


    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        //proc = GetCurrentProcess;
        procId = GetCurrentProcessId();
        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
        if (NULL != hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseName(hProcess, hMod, szProcessName,
                    sizeof(szProcessName) / sizeof(TCHAR));
            }
        }

        if (lstrcmpiW((LPCWSTR)szProcessName, (LPCWSTR)TEXT("blackops3.exe")) == 0)
        {
            hModule = hModule_;
            procId = GetProcessId(hModule); 
            CreateThread(0, 0, &Init, 0, 0, 0);
        }
    }

    return TRUE;
}

