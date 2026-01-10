#include "zi_platform.h"
#include "zi_common.h"

#ifdef ZI_EMSCRIPTEN

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>


f64 zi_platform_get_time(void) {
	return emscripten_get_now() / 1000.0;
}

void zi_platform_console_log(const char* message, i32 len, u8 error) {
	if (error) {
		emscripten_console_error(message);
	} else {
		emscripten_console_log(message);
	}
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
	return ZiGraphicsBackend_WebGPU;
}

void zi_app_init();
void zi_app_loop();
void zi_app_terminate();

static void zi_web_loop(void *user_data) {
	zi_app_loop();
}

static void zi_web_app_shutdown() {
	zi_app_terminate();
}

int main(void) {
	zi_app_init();
	atexit(zi_web_app_shutdown);
	emscripten_set_main_loop_arg(zi_web_loop, NULL, 0, 0);
	return 0;
}

#endif
