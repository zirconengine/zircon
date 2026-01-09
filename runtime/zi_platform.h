#pragma once

#include "zi_common.h"
#include "zi_graphics.h"

ZI_HANDLER(ZiWindow);

enum ZiWindowFlags_ {
	ZiWindowFlags_None      = 0,
	ZiWindowFlags_Maximized = 1 << 0,
};

typedef u32 ZiWindowFlags;

enum ZiCursorMode_ {
	ZiCursorMode_Normal   = 0,
	ZiCursorMode_Hidden   = 1,
	ZiCursorMode_Disabled = 2,
};

typedef u32 ZiCursorMode;

void zi_platform_init();
void zi_platform_terminate();

ZiWindow zi_platform_create_window(const char* title, i32 width, i32 height, ZiWindowFlags flags);
void     zi_platform_destroy_window(ZiWindow window);
void     zi_platform_poll_events();
i8       zi_platform_should_close(ZiWindow window);

// Window queries
void zi_platform_get_window_size(ZiWindow window, i32* width, i32* height);
void zi_platform_get_framebuffer_size(ZiWindow window, i32* width, i32* height);
void zi_platform_set_window_title(ZiWindow window, const char* title);

// Window state
void zi_platform_set_window_visible(ZiWindow window, i8 visible);
i8   zi_platform_is_window_focused(ZiWindow window);
i8   zi_platform_is_window_minimized(ZiWindow window);

// Fullscreen
void zi_platform_set_fullscreen(ZiWindow window, i8 fullscreen);
i8   zi_platform_is_fullscreen(ZiWindow window);

// Cursor
void zi_platform_set_cursor_mode(ZiWindow window, ZiCursorMode mode);
ZiCursorMode zi_platform_get_cursor_mode(ZiWindow window);

// Time
f64 zi_platform_get_time(void);

// Native handle (for Vulkan surface creation)
VoidPtr zi_platform_get_native_handle(ZiWindow window);

// Console/logging
void zi_platform_console_log(const char* message, i32 len, u8 error);

// Time formatting (writes to buf, returns chars written, format: "YYYY-MM-DD HH:MM:SS:mmm")
i32 zi_platform_get_timestamp(char* buf, i32 buf_size);

ZiGraphicsBackend zi_platform_get_graphics_backend(ZiGraphicsBackend backend);
