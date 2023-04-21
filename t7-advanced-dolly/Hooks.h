#pragma once
#include <Psapi.h>
#include <Windows.h>
#include <list>
#include "Dolly.h"
#include "Addresses.h"

#define FOV_Limit 90.0
#define Roll_Increment 5.0
#define Focus_Increment 10.0
#define Aperture_Increment 0.5

// ...Dont judge me...
struct RegisterStore {
	DWORD64 rax;
	DWORD64 rcx;
	DWORD64 rdx;
	DWORD64 rbx;
	DWORD64 rbp;
	DWORD64 rsi;
	DWORD64 rdi;
	DWORD64 rsp;
	DWORD64 rip;
	DWORD64 r8;
	DWORD64 r9;
	DWORD64 r10;
	DWORD64 r11;
	DWORD64 r12;
	DWORD64 r13;
	DWORD64 r14;
	DWORD64 r15;
};

struct Patch {
	void* address;
	const char* bytes[50];
	int length;
};

int patchcount = 0;
Patch* patches[50];

MSG msg;
DWORD current_process = 0;

bool Hooked = false;

bool get_state() {
	if (GetMessage(&msg, GetActiveWindow(), 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {

		if (*(DWORD64*)Addresses::TheaterBasePointer != 0)
		{

			if (wParam == WM_MOUSEWHEEL)
			{
				MSLLHOOKSTRUCT* pMhs = (MSLLHOOKSTRUCT*)lParam;
				short zDelta = HIWORD(pMhs->mouseData);
				HWND foreground = GetForegroundWindow();
				DWORD foregroundID = 0;
				GetWindowThreadProcessId(foreground, &foregroundID);

				if (foregroundID == current_process)
				{

					//Increment gets lower as the fov gets lower, and vice versa
					double inc = min(max(pow(*Addresses::FOV, 1.5) * 0.02, 0.1), 3.0);

					if (zDelta < 0)
					{
						//Down

						if (GetKeyState(VK_SHIFT) & 0x8000)
						{
							if (*Addresses::FOV + inc < FOV_Limit)
							{
								*Addresses::FOV += inc;
							}
							else {
								*Addresses::FOV = FOV_Limit;
							}
						}
						else if (GetKeyState(VK_CONTROL) & 0x8000) {
							if (*Addresses::FocalDistance - Focus_Increment > 0)
								*Addresses::FocalDistance -= Focus_Increment;
						}
						else if (GetKeyState(VK_MENU) & 0x8000)
						{
							if (*Addresses::Aperture - Aperture_Increment > 0)
							{
								*Addresses::Aperture -= Aperture_Increment;
							}
						}
						else {
							Settings::roll += Roll_Increment;
						}

					}
					else {
						//Up
						if (GetKeyState(VK_SHIFT) & 0x8000)
						{
							if (*Addresses::FOV - inc > 0.1f) {
								*Addresses::FOV -= inc;
							}
							else {
								*Addresses::FOV = 0.1;
							}
						}
						else if (GetKeyState(VK_CONTROL) & 0x8000) {
							*Addresses::FocalDistance += Focus_Increment;
						}
						else if (GetKeyState(VK_MENU) & 0x8000)
						{
							*Addresses::Aperture += Aperture_Increment;
						}
						else {
							Settings::roll -= Roll_Increment;
							//*Addresses::Roll -= Roll_Increment;
						}
					}
				}
			}
			else if (wParam == WM_MBUTTONDOWN)
			{
				HWND foreground = GetForegroundWindow();
				DWORD foregroundID = 0;
				GetWindowThreadProcessId(foreground, &foregroundID);

				if (foregroundID == current_process)
				{
					if (GetKeyState(VK_SHIFT) & 0x8000)
					{
						*Addresses::FOV = 14.0;
					}
					else {
						Settings::roll = 0;
					}
				}
			}

		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

DWORD WINAPI MouseHook(LPVOID param)
{
	HHOOK mousehook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	while (true)
	{
		get_state();
		Sleep(20);
		UnhookWindowsHookEx(mousehook);
		return 0;
	}
}

DWORD64 CreateHook(void* toHook, void* hk_func, int len) {
	if (len < 14) {                                            //if less than 13 bytes
		return 0;                                         //we gtfo
	}

	Hooked = true;

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	//copy the original bytes, for restoring
	patches[patchcount] = new Patch();

	memcpy(patches[patchcount]->bytes, toHook, len);
	patches[patchcount]->length = len;
	patches[patchcount]->address = toHook;
	patchcount++;

	memset(toHook, 0x90, len);

	unsigned char patch[] = {
		0xFF, 0x25,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //Address goes here
	};
	*(DWORD64*)&patch[6] = (DWORD64)hk_func; //replacing zeros with our function address

	memcpy((void*)toHook, patch, sizeof(patch));
	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);
	return((DWORD64)toHook) + len;
}

void RestoreHooks() {

	for (int i = 0; i < patchcount; i++)
	{

		Log::Debug("Restoring: %llx. Length: %d", patches[i]->address, patches[i]->length);

		DWORD curProtection;
		VirtualProtect(patches[i]->address, patches[i]->length, PAGE_EXECUTE_READWRITE, &curProtection);

		memcpy(patches[i]->address, &patches[i]->bytes, patches[i]->length);

		DWORD temp;
		VirtualProtect(patches[i]->address, patches[i]->length, curProtection, &temp);
	}

	patchcount = 0;
	Hooked = false;
}

RegisterStore CreateDollyRegisters;
DWORD64 CreateDollyReturn;

void __declspec(naked) CreateDolly_Hook() {

	// Listen... I know this is stupid
	// But it works!
	__asm {
		mov CreateDollyRegisters.rax, rax
		mov CreateDollyRegisters.rcx, rcx
		mov CreateDollyRegisters.rdx, rdx
		mov CreateDollyRegisters.rbx, rbx
		mov CreateDollyRegisters.rbp, rbp
		mov CreateDollyRegisters.rsi, rsi
		mov CreateDollyRegisters.rdi, rdi
		mov CreateDollyRegisters.rsp, rsp
		mov CreateDollyRegisters.rip, rip
		mov CreateDollyRegisters.r8, r8
		mov CreateDollyRegisters.r9, r9
		mov CreateDollyRegisters.r10, r10
		mov CreateDollyRegisters.r11, r11
		mov CreateDollyRegisters.r12, r12
		mov CreateDollyRegisters.r13, r13
		mov CreateDollyRegisters.r14, r14
		mov CreateDollyRegisters.r15, r15
	}
	//Recreate the original code. i think it moves the current tick into the next dolly marker
	*(int*)(CreateDollyRegisters.rbx + CreateDollyRegisters.rdi + 0x00983248) = (int)CreateDollyRegisters.rax;
	CreateDollyRegisters.rcx = Addresses::THEATER_BASE;

	Log::Info("Marker Address: %llx    DollyMarkers: %llx", CreateDollyRegisters.rbx + CreateDollyRegisters.rdi + 0x00983248, Addresses::DollyMarkers);

	Dolly::CreateDollyCam(CreateDollyRegisters.rbx + CreateDollyRegisters.rdi + 0x00983248);

	

	__asm {
		mov rdi, CreateDollyRegisters.rdi
		mov rsi, CreateDollyRegisters.rsi
		mov rbp, CreateDollyRegisters.rbp
		mov rbx, CreateDollyRegisters.rbx
		mov rdx, CreateDollyRegisters.rdx
		mov rcx, CreateDollyRegisters.rcx
		mov rax, CreateDollyRegisters.rax
		mov rsp, CreateDollyRegisters.rsp
		mov rip, CreateDollyRegisters.rip
		mov r8,  CreateDollyRegisters.r8
		mov r9,  CreateDollyRegisters.r9
		mov r10, CreateDollyRegisters.r10
		mov r11, CreateDollyRegisters.r11
		mov r12, CreateDollyRegisters.r12
		mov r13, CreateDollyRegisters.r13
		mov r14, CreateDollyRegisters.r14
		mov r15, CreateDollyRegisters.r15
		jmp[CreateDollyReturn]
	}
}

RegisterStore EditDollyRegisters;
DWORD64 EditDollyReturn;

void __declspec(naked) EditDolly_Hook() {
	__asm {
		mov EditDollyRegisters.rax, rax
		mov EditDollyRegisters.rcx, rcx
		mov EditDollyRegisters.rdx, rdx
		mov EditDollyRegisters.rbx, rbx
		mov EditDollyRegisters.rbp, rbp
		mov EditDollyRegisters.rsi, rsi
		mov EditDollyRegisters.rdi, rdi
		mov EditDollyRegisters.rsp, rsp
		mov EditDollyRegisters.rip, rip
		mov EditDollyRegisters.r8, r8
		mov EditDollyRegisters.r9, r9
		mov EditDollyRegisters.r10, r10
		mov EditDollyRegisters.r11, r11
		mov EditDollyRegisters.r12, r12
		mov EditDollyRegisters.r13, r13
		mov EditDollyRegisters.r14, r14
		mov EditDollyRegisters.r15, r15
	}

	//Recreate the original code
	*(int*)(EditDollyRegisters.rbx + EditDollyRegisters.rcx + 0x00983250) = (int)EditDollyRegisters.rax;
	EditDollyRegisters.rax = *(int*)(EditDollyRegisters.rcx + 0x981A6C);
	EditDollyRegisters.r8 = 0;

	Dolly::EditDollyCam(EditDollyRegisters.rbx + EditDollyRegisters.rcx + 0x00983250 - 8);

	__asm {
		mov rdi, EditDollyRegisters.rdi
		mov rsi, EditDollyRegisters.rsi
		mov rbp, EditDollyRegisters.rbp
		mov rbx, EditDollyRegisters.rbx
		mov rdx, EditDollyRegisters.rdx
		mov rcx, EditDollyRegisters.rcx
		mov rax, EditDollyRegisters.rax
		mov rsp, EditDollyRegisters.rsp
		mov rip, EditDollyRegisters.rip
		mov r8,  EditDollyRegisters.r8
		mov r9,  EditDollyRegisters.r9
		mov r10, EditDollyRegisters.r10
		mov r11, EditDollyRegisters.r11
		mov r12, EditDollyRegisters.r12
		mov r13, EditDollyRegisters.r13
		mov r14, EditDollyRegisters.r14
		mov r15, EditDollyRegisters.r15
		jmp[EditDollyReturn]
	}
}

RegisterStore RollRegisters;
DWORD64 RollReturn;
void __declspec(naked) Roll_Hook() {
	__asm {
		mov RollRegisters.rax, rax
		mov RollRegisters.rcx, rcx
		mov RollRegisters.rdx, rdx
		mov RollRegisters.rbx, rbx
		mov RollRegisters.rbp, rbp
		mov RollRegisters.rsi, rsi
		mov RollRegisters.rdi, rdi
		mov RollRegisters.rsp, rsp
		mov RollRegisters.rip, rip
		mov RollRegisters.r8, r8
		mov RollRegisters.r9, r9
		mov RollRegisters.r10, r10
		mov RollRegisters.r11, r11
		mov RollRegisters.r12, r12
		mov RollRegisters.r13, r13
		mov RollRegisters.r14, r14
		mov RollRegisters.r15, r15
	}

	Dolly::Loop();
	*RollValue = Settings::roll;

	__asm {
		mov rdi, RollRegisters.rdi
		mov rsi, RollRegisters.rsi
		mov rbp, RollRegisters.rbp
		mov rbx, RollRegisters.rbx
		mov rdx, RollRegisters.rdx
		mov rcx, RollRegisters.rcx
		mov rax, RollRegisters.rax
		mov rsp, RollRegisters.rsp
		mov rip, RollRegisters.rip
		mov r8,  RollRegisters.r8
		mov r9,  RollRegisters.r9
		mov r10, RollRegisters.r10
		mov r11, RollRegisters.r11
		mov r12, RollRegisters.r12
		mov r13, RollRegisters.r13
		mov r14, RollRegisters.r14
		mov r15, RollRegisters.r15
		jmp[RollReturn]
	}
}


void CreateHooks_Internal() {
	CreateDollyReturn = CreateHook(Addresses::CreateDollyHook, &CreateDolly_Hook, 14);
	EditDollyReturn = CreateHook(Addresses::EditDollyHook, &EditDolly_Hook, 16);
	RollReturn = CreateHook(Addresses::RollHookAddress, &Roll_Hook, 17);
}

DWORD WINAPI GameCheckThread(LPVOID params)
{
	while (true)
	{
		if (Hooked)
		{
			//If hooked and not in theater mode
			if (*(DWORD64*)Addresses::TheaterBasePointer == 0)
			{
				//Restore Everything
				*Addresses::FOV = 14.0;
				RestoreHooks();
				MessageBoxA(0, "Theater mode exit detected. Do not join any lobbies until you restart the game.", "Advanced Dolly Cams", 0);
			}
		}
		else {
			//If not hooked, and in theater mode
			if (*(DWORD64*)Addresses::TheaterBasePointer != 0)
			{
				//Recreate Hooks
				Addresses::ReadAllAddresses();
				CreateHooks_Internal();
			}
		}

		Sleep(2000);
	}
	return 0;
}

namespace Hooks {

	void Initialise() {
		
		current_process = GetCurrentProcessId();
		
		CreateThread(NULL, 0, &MouseHook, NULL, 0, NULL);
		CreateThread(NULL, 0, &GameCheckThread, NULL, 0, NULL);
	}
}

