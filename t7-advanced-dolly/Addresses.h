#pragma once
#include "StdInc.h"
#include "Memory.h"

namespace Addresses {
	DWORD64 BASE_ADDRESS;
	DWORD64 THEATER_BASE;

	DWORD64 TheaterBasePointer;

	//float* Roll;
	float* RollValue;
	float* FocalDistance;
	float* Aperture;
	double* FOV;

	void* CreateDollyHook;
	void* EditDollyHook;
	void* RollHookAddress;

	int* DollyCameraMode;
	int* Tick;
	int* NumDollyCams;

	DollyMarker *DollyMarkers;

	DWORD64 ReadPointer(DWORD64 base, DWORD64 offset)
	{
		return (*(DWORD64*)base) + offset;
	}

	void ReadAllAddresses() {
		BASE_ADDRESS = Memory::GetBaseAddress(L"BlackOps3.exe");
		Log::Info("Base Address: %llx\r\n", BASE_ADDRESS);

		FOV = (double*)Memory::Scan(L"BlackOps3.exe", "\x00\x00\x00\x00\x00\x00\x2C\x40\x66\x66\x66\x66\x66\x66\x39\x40", "????????xxxxxxxx");
		Log::Info("FOV: %llx\r\n", FOV);


		/*					Roll Hook Address
			This code gets called every frame, and sets the roll to zero
			We can hook it to overwrite the roll value, and also call the rest of our dolly loop per frame
		 
			E8 99 17 04 00						call    sub_7FF7A1A420C0
			84 C0								test    al, al
			75 1A								jnz     short loc_7FF7A1A00945
			E8 C0 17 04 00						call    sub_7FF7A1A420F0
			84 C0								test    al, al
			75 11								jnz     short loc_7FF7A1A00945
	>>>>	48 8B 05 C5 D0 21 17				mov     rax, cs : qword_7FF7B8C1DA00
			C7 80 7C 1A 98 00 00 00 00 00		mov     dword ptr[rax + 981A7Ch], 0
		*/

		RollHookAddress = (void*) (Memory::Scan(L"BlackOps3.exe", "\x48\x8b\x05\x00\x00\x00\x00\xc7\x80\x00\x00\x00\x00\x00\x00\x00\x00\x8b\xcf", "xxx????xx????????xx"));
		Log::Info("RollHookAddress: %llx\r\n", RollHookAddress);


		/*					Create Dolly Hook
			This code gets called every time a new dolly marker is created
			We hook here so we can store extra info for the new dolly marker

			48 63 9F D8 45 98 00				movsxd  rbx, dword ptr[rdi + 9845D8h]
			45 33 C9							xor r9d, r9d
			45 33 C0							xor r8d, r8d
			48 6B DB 64							imul    rbx, 64h
	>>>>	89 84 3B 48 32 98 00				mov[rbx + rdi + 983248h], eax
			48 8B 0D B1 F7 1E 17				mov     rcx, cs : qword_7FF7B8C1DA00	<<<< This qword is also the address of TheaterBase >:)
			8B 81 68 1A 98 00					mov     eax, [rcx + 981A68h]
		*/

		CreateDollyHook = (void*)Memory::Scan(L"BlackOps3.exe", "\x89\x84\x3b\x00\x00\x00\x00\x48\x8b\x0d\x00\x00\x00\x00\x8b\x81", "xxx????xxx????xx");
		Log::Info("CreateDollyHook: %llx\r\n", CreateDollyHook);

		DWORD64 DollyTemp = (DWORD64)CreateDollyHook;
		DWORD64 TheaterBaseOffset = *(int*)(DollyTemp + 0xA);
		DWORD64 TheaterBase = DollyTemp + TheaterBaseOffset + 0xE;
		int DollyCameraOffset = *(int*)(DollyTemp + 3);

		while (*(DWORD64*)TheaterBase == 0)
		{
			Sleep(2000);
			Log::Info("Waiting for theater mode... Theater Base: %llx, Dolly Offset: %llx", TheaterBase, DollyCameraOffset);
		}

		THEATER_BASE = *(DWORD64*)TheaterBase;
		Log::Info("THEATER_BASE: %llx\r\n", THEATER_BASE);

		DollyMarkers = (DollyMarker*)((*(DWORD64*)TheaterBase) + DollyCameraOffset);
		Log::Info("DollyMarkers: %llx\r\n", DollyMarkers);

		DollyCameraMode = (int*)((*(DWORD64*)TheaterBase) + 0x7DAE64);
		Log::Info("DollyCameraMode: %llx\r\n", DollyCameraMode);

		NumDollyCams = (int*)((*(DWORD64*)TheaterBase) + 0x9845D8);
		Log::Info("NumDollyCams: %llx\r\n", NumDollyCams);

		RollValue = (float*)((*(DWORD64*)TheaterBase) + 0x981A7C);
		Log::Info("RollValue: %llx\r\n", RollValue);

		TheaterBasePointer = TheaterBase;
		Log::Info("EditDollyHookAddress = %llx", EditDollyHook);
		

		/*					Edit Dolly Hook
			This code gets called every time a dolly marker is edited
			We hook here so we can update our extra data

			8B 81 68 1A 98 00					mov     eax, [rcx+981A68h]
			48 6B DB 64							imul    rbx, 64h
			45 33 C9							xor     r9d, r9d
	>>>>	89 84 0B 50 32 98 00				mov     [rbx+rcx+983250h], eax
			8B 81 6C 1A 98 00					mov     eax, [rcx+981A6Ch]
			45 33 C0							xor     r8d, r8d
			89 84 0B 54 32 98 00				mov     [rbx+rcx+983254h], eax
			8B 81 70 1A 98 00					mov     eax, [rcx+981A70h]
			89 84 0B 58 32 98 00

			Honestly shocked this sig works...
		*/ 

		EditDollyHook = (void*)Memory::Scan(L"BlackOps3.exe", "\x89\x84\x0b\x00\x00\x00\x00\x8b\x81\x00\x00\x00\x00\x45\x33\xc0\x89\x84\x0b\x00\x00\x00\x00\x8b\x81\x00\x00\x00\x00\x89\x84\x0b\x00\x00\x00\x00\x48\x8b\x0d\x00\x00\x00\x00\x48\x81\xc1\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x48\x8b\x05\x00\x00\x00\x00\xf3\x0f\x10\x44\x24\x00\xf3\x0f\x10\x15\x00\x00\x00\x00\xf3\x0f\x59\xc2\xf3\x0f\x58\x84\x03\x00\x00\x00\x00\xf3\x0f\x11\x84\x03\x00\x00\x00\x00\xf3\x0f\x10\x44\x24\x00\xf3\x0f\x59\xc2\xf3\x0f\x58\x84\x03\x00\x00\x00\x00\xf3\x0f\x11\x84\x03\x00\x00\x00\x00\xf3\x0f\x10\x4c\x24\x00\xf3\x0f\x59\xca\xf3\x0f\x58\x8c\x03\x00\x00\x00\x00\xf3\x0f\x11\x8c\x03\x00\x00\x00\x00\xeb\x00\x48\x8b\x15\x00\x00\x00\x00\x48\x8b\xcf\x48\x6b\xc9", "xxx????xx????xxxxxx????xx????xxx????xxx????xxx????x????xxx????xxxxx?xxxx????xxxxxxxxx????xxxxx????xxxxx?xxxxxxxxx????xxxxx????xxxxx?xxxxxxxxx????xxxxx????x?xxx????xxxxxx");
		Log::Info("EditDollyHook: %llx\r\n", EditDollyHook);


		/*	Just a pointer to the camera dof structure

			8B 44 24 64							mov     eax, [rsp+380h+var_31C]
			89 06								mov     [rsi], eax
	>>>>	48 8B 0D 73 4F 5A 0A				mov     rcx, cs:off_7FF7AA2C7930
			E8 1E 43 9D 01						call    sub_7FF7A16F6CE0
			48 8D 8E 88 89 2D 00				lea     rcx, [rsi+2D8988h]

		*/

		DWORD64 CameraBasePointer = Memory::Scan(L"BlackOps3.exe", "\x48\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x8E\x00\x00\x00\x00", "xxx????x????xxx????");
		Log::Info("CameraBasePointer: %llx\r\n", CameraBasePointer);

		int cameraPointerOffset = *(int*)(CameraBasePointer + 3);
		Log::Info("cameraOffset: %x\r\n", cameraPointerOffset);

		DWORD64 cameraAddress = *(DWORD64*)(CameraBasePointer + cameraPointerOffset + 0x7);
		DWORD64 cameraBase = cameraAddress + 0x30;

		Log::Info("cameraBase: %llx\r\n", cameraBase);


		FocalDistance = (float*)ReadPointer(cameraBase, 0x131d30);
		Log::Info("FocalDistance: %llx\r\n", FocalDistance);

		Aperture = (float*)ReadPointer(cameraBase, 0x131D34);
		Log::Info("Aperture: %llx\r\n", Aperture);



		/*	Sig to get a pointer to tick address
		
			8B 05 DC DB 04 01					mov     eax, dword ptr cs:qword_7FF7A287A598+4
	>>>>	8B FB								mov     edi, ebx
			89 05 D0 DB 04 01					mov     dword ptr cs:qword_7FF7A287A598, eax	<<<< This is the ptr address we want
			89 1D CE DB 04 01					mov     dword ptr cs:qword_7FF7A287A598+4, ebx
			2B F8								sub     edi, eax
			74 48								jz      short loc_7FF7A182CA1A
		*/

		DWORD64 TickScanResult = Memory::Scan(L"BlackOps3.exe", "\x8B\xFB\x89\x05\x00\x00\x00\x00", "xxxx????");
		Log::Info("TickScanResult = %llx", TickScanResult);

		int tickOffset = *(int*)(TickScanResult + 4);
		Log::Info("tickOffset = %x", tickOffset);

		Tick = (int*)(TickScanResult + tickOffset + 8);
		Log::Info("Tick: %llx\r\n", Tick);
	}
}