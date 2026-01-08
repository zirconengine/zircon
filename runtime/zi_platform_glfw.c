#include "zi_platform.h"

#include <GLFW/glfw3.h>

void zi_platform_init() { glfwInit(); }

void zi_platform_terminate() { glfwTerminate(); }

ZiWindow zi_platform_create_window(const char *title, i32 width, i32 height,
                                   ZiWindowFlags flags) {

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwWindowHint(GLFW_MAXIMIZED,
                 flags & ZiWindowFlags_Maximized ? GLFW_TRUE : GLFW_FALSE);

  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    // TODO - log error
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