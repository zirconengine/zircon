#include "zi_log.h"

#include "zi_platform.h"

#include <stdarg.h>
#include <stdio.h>


static const char* level_desc[] = {"trace", "debug", "info", "warn", "error", "critical", "off"};

void zi_log(ZiLogLevel level, const char* fmt, ...) {
	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	char buffer2[1024];
	i32  size = snprintf(buffer2, sizeof(buffer2), "[%s] %s\n", level_desc[level], buffer);
	zi_platform_console_log(buffer2, size, level >= ZiLogLevel_Error);
}
