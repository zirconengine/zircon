#include "zi_graphics.h"
#include "zi_platform.h"

int main() {

  zi_platform_init();
  zi_graphics_init(0);

  ZiWindow window = zi_platform_create_window("Zircon Engine", 800, 600, ZiWindowFlags_Maximized);

  while (!zi_platform_should_close(window)) {
    zi_platform_poll_events();
  }

  zi_platform_destroy_window(window);

  zi_graphics_terminate();
  zi_platform_terminate();
}