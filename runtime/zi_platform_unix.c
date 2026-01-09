#include "zi_platform.h"

#include "zi_common.h"

#if defined(ZI_LINUX) || defined(ZI_MACOS)

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


void zi_platform_console_log(const char* message, i32 len, u8 error) {
	int fd = error ? STDERR_FILENO : STDOUT_FILENO;
	write(fd, message, len);
}

i32 zi_platform_get_timestamp(char* buf, i32 buf_size) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct tm tm_info;
	localtime_r(&tv.tv_sec, &tm_info);

	i32 ms = tv.tv_usec / 1000;
	return snprintf(buf, buf_size, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
	                tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
	                tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms);
}

ZiGraphicsBackend zi_platform_get_graphics_backend(ZiGraphicsBackend backend) {
#ifdef ZI_MACOS
	return ZiGraphicsBackend_Metal;
#else
	return ZiGraphicsBackend_Vulkan;
#endif
}

#endif
