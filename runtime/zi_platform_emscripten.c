#include "zi_platform.h"
#include "zi_common.h"

#ifdef ZI_EMSCRIPTEN

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

void zi_platform_init() {}
void zi_platform_terminate() {}

ZiWindow zi_platform_create_window(const char* title, i32 width, i32 height, ZiWindowFlags flags) { return (ZiWindow){0}; }
void     zi_platform_destroy_window(ZiWindow window) {}
void     zi_platform_poll_events() {}
i8       zi_platform_should_close(ZiWindow window) {
	return 1;
}

void zi_platform_console_log(const char* message, i32 len, u8 error) {
	if (error) {
		emscripten_console_error(message);
	} else {
		emscripten_console_log(message);
	}
}

ZiGraphicsBackend zi_platform_get_graphics_backend(ZiGraphicsBackend backend) {
	return ZiGraphicsBackend_WebGPU;
}

#endif
