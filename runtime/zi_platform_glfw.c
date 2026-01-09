#include "zi_platform.h"
#include "zi_log.h"

#if ZI_DESKTOP
#include <GLFW/glfw3.h>

#ifdef ZI_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>


void zi_platform_init() { glfwInit(); }

void zi_platform_terminate() { glfwTerminate(); }

ZiWindow zi_platform_create_window(const char* title, i32 width, i32 height, ZiWindowFlags flags) {
	glfwDefaultWindowHints();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, flags & ZiWindowFlags_Maximized ? GLFW_TRUE : GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		zi_log_error("Failed to create GLFW window");
		return (ZiWindow){0};
	}

	return (ZiWindow){.handler = window};
}

void zi_platform_destroy_window(ZiWindow window) {
	glfwDestroyWindow(window.handler);
}

void zi_platform_poll_events() {
	glfwPollEvents();
}

i8 zi_platform_should_close(ZiWindow window) {
	return (i8)glfwWindowShouldClose(window.handler);
}

void zi_platform_get_window_size(ZiWindow window, i32* width, i32* height) {
	glfwGetWindowSize(window.handler, width, height);
}

void zi_platform_get_framebuffer_size(ZiWindow window, i32* width, i32* height) {
	glfwGetFramebufferSize(window.handler, width, height);
}

void zi_platform_set_window_title(ZiWindow window, const char* title) {
	glfwSetWindowTitle(window.handler, title);
}

void zi_platform_set_window_visible(ZiWindow window, i8 visible) {
	if (visible) {
		glfwShowWindow(window.handler);
	} else {
		glfwHideWindow(window.handler);
	}
}

i8 zi_platform_is_window_focused(ZiWindow window) {
	return (i8)glfwGetWindowAttrib(window.handler, GLFW_FOCUSED);
}

i8 zi_platform_is_window_minimized(ZiWindow window) {
	return (i8)glfwGetWindowAttrib(window.handler, GLFW_ICONIFIED);
}

void zi_platform_set_fullscreen(ZiWindow window, i8 fullscreen) {
	if (fullscreen) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window.handler, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		glfwSetWindowMonitor(window.handler, NULL, 100, 100, 1280, 720, 0);
	}
}

i8 zi_platform_is_fullscreen(ZiWindow window) {
	return glfwGetWindowMonitor(window.handler) != NULL;
}

void zi_platform_set_cursor_mode(ZiWindow window, ZiCursorMode mode) {
	int glfw_mode = GLFW_CURSOR_NORMAL;
	switch (mode) {
		case ZiCursorMode_Normal:   glfw_mode = GLFW_CURSOR_NORMAL; break;
		case ZiCursorMode_Hidden:   glfw_mode = GLFW_CURSOR_HIDDEN; break;
		case ZiCursorMode_Disabled: glfw_mode = GLFW_CURSOR_DISABLED; break;
	}
	glfwSetInputMode(window.handler, GLFW_CURSOR, glfw_mode);
}

ZiCursorMode zi_platform_get_cursor_mode(ZiWindow window) {
	int mode = glfwGetInputMode(window.handler, GLFW_CURSOR);
	switch (mode) {
		case GLFW_CURSOR_HIDDEN:   return ZiCursorMode_Hidden;
		case GLFW_CURSOR_DISABLED: return ZiCursorMode_Disabled;
		default:                   return ZiCursorMode_Normal;
	}
}

f64 zi_platform_get_time(void) {
	return glfwGetTime();
}

VoidPtr zi_platform_get_native_handle(ZiWindow window) {
#ifdef ZI_WIN
	return (VoidPtr)glfwGetWin32Window(window.handler);
#elif defined(__APPLE__)
	return (VoidPtr)glfwGetCocoaWindow(window.handler);
#elif defined(__linux__)
	return (VoidPtr)glfwGetX11Window(window.handler);
#else
	return NULL;
#endif
}

#endif