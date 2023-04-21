#include "pch.h"
#include "Log.h"
#include "StdInc.h"
#include <ctime>

#define buffer_size 100
#define lpcwPipe L"\\\\.\\pipe\\T7xADC"

void Log::Print(logChannel channel, const char* format, va_list args) {

	while (queue > 0)Sleep(1);

	queue++;

	char line[buffer_size];
	vsnprintf(line, buffer_size, format, args);



	//char line2[buffer_size];
	//snprintf(line, buffer_size, "%d]%s", channel);
	//_cprintf(" %s] ", channelNames[channel]);

	std::string text = std::to_string(channel) + std::string(line);

	HANDLE hPipe = CreateFile(lpcwPipe, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!(hPipe == NULL || hPipe == INVALID_HANDLE_VALUE))
	{
		WriteFile(hPipe, text.c_str(), text.size(), 0, NULL);
		CloseHandle(hPipe);
		Sleep(10);
	}

	queue--;
}

void Log::Info(const char* format, ...) {
	va_list args;
	va_start(args, format);
	Print(logChannel::Info, format, args);
	va_end(args);
}

void Log::Error(const char* format, ...) {
	va_list args;
	va_start(args, format);
	Print(logChannel::Error, format, args);
	va_end(args);
}

void Log::Debug(const char* format, ...) {
	va_list args;
	va_start(args, format);
	Print(logChannel::Debug, format, args);
	va_end(args);
}

void Log::Init()
{
	const char* text = "Connected to T7";

	HANDLE hPipe = CreateFile(lpcwPipe, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE) {
		MessageBoxA(0, "Could not open the pipe", "Error", 0);
	}
	else {
		WriteFile(hPipe, text, strlen(text), 0, NULL);
		CloseHandle(hPipe);
	}
}

void Log::DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		_cprintf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		}
		else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			_cprintf(" ");
			if ((i + 1) % 16 == 0) {
				_cprintf("|  %s \n", ascii);
			}
			else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					_cprintf(" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					_cprintf("   ");
				}
				_cprintf("|  %s \n", ascii);
			}
		}
	}
}

uint32_t Log::queue = 0;