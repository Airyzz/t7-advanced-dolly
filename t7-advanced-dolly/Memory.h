#pragma once
#include <Psapi.h>
#include "StdInc.h"

char* ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size)
{
    unsigned int patternLength = strlen(mask);

    for (unsigned int i = 0; i < size - patternLength; i++)
    {
        bool found = true;
        for (unsigned int j = 0; j < patternLength; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (begin + i);
        }
    }
    return nullptr;
}

namespace Memory {
	
	DWORD64 GetBaseAddress(LPCWSTR modName) {
		HMODULE mod = GetModuleHandleW(modName);
		MODULEINFO info = MODULEINFO();
		GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(MODULEINFO));
		return (DWORD64)info.lpBaseOfDll;
	}

    DWORD64 Scan(LPCWSTR modName, const char* pattern, const char* mask)
    {
        HMODULE mod = GetModuleHandleW(modName);
        MODULEINFO info = MODULEINFO();
        GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(MODULEINFO));
        return (DWORD64)ScanIn(pattern, mask, (char*)info.lpBaseOfDll, info.SizeOfImage);
    }


}
