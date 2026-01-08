#include "zi_platform.h"

#include "zi_common.h"

#ifdef ZI_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


void zi_platform_console_log(const char* message, i32 len, u8 error) {
	HANDLE h = error ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(h, message, len, NULL, NULL);

	//FlushFileBuffers(h);
}

#endif
