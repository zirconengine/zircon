#include "zi_platform.h"

#include "zi_common.h"

#ifdef ZI_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>


void zi_platform_console_log(const char* message, i32 len, u8 error) {
	HANDLE h = error ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(h, message, len, NULL, NULL);

	//FlushFileBuffers(h);
}

i32 zi_platform_get_timestamp(char* buf, i32 buf_size) {
	SYSTEMTIME st;
	GetLocalTime(&st);
	return snprintf(buf, buf_size, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
	                st.wYear, st.wMonth, st.wDay,
	                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

ZiGraphicsBackend zi_platform_get_graphics_backend(ZiGraphicsBackend backend) {
	return ZiGraphicsBackend_Vulkan;
}

i32 zi_platform_run(int argc, char** argv);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int     argc;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

	char** argv = malloc(sizeof(char*) * argc);
	for (int i = 0; i < argc; i++) {
		int size = WideCharToMultiByte(CP_ACP, 0, argvW[i], -1, NULL, 0, NULL, NULL);
		argv[i] = malloc(sizeof(char) * size);
		WideCharToMultiByte(CP_ACP, 0, argvW[i], -1, argv[i], size, NULL, NULL);
	}

	i32 res = zi_platform_run(argc, argv);

	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}

	free(argv);
	LocalFree(argvW);

	return res;
}

#endif
