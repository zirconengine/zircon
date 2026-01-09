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
i8 zi_platform_should_close(ZiWindow window) {
	return 1;
}

void zi_platform_get_window_size(ZiWindow window, i32* width, i32* height) {
	*width = 0;
	*height = 0;
}

void zi_platform_get_framebuffer_size(ZiWindow window, i32* width, i32* height) {
	*width = 0;
	*height = 0;
}

void zi_platform_set_window_title(ZiWindow window, const char* title) {}

void zi_platform_set_window_visible(ZiWindow window, i8 visible) {}

i8 zi_platform_is_window_focused(ZiWindow window) {
	return 1;
}

i8 zi_platform_is_window_minimized(ZiWindow window) {
	return 0;
}

void zi_platform_set_fullscreen(ZiWindow window, i8 fullscreen) {}

i8 zi_platform_is_fullscreen(ZiWindow window) {
	return 0;
}

void zi_platform_set_cursor_mode(ZiWindow window, ZiCursorMode mode) {}

ZiCursorMode zi_platform_get_cursor_mode(ZiWindow window) {
	return ZiCursorMode_Normal;
}

f64 zi_platform_get_time(void) {
	return emscripten_get_now() / 1000.0;
}

VoidPtr zi_platform_get_native_handle(ZiWindow window) {
	return NULL;
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
