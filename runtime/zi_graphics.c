#include "zi_graphics.h"

#include "zi_log.h"

void zi_graphics_init_vulkan(ZiRenderDevice* device);

static ZiRenderDevice device = {};

void zi_graphics_init(ZiGraphicsBackend backend) {
  switch (backend) {
  case ZiGraphicsBackend_Vulkan:
    zi_graphics_init_vulkan(&device);
    break;
  case ZiGraphicsBackend_Metal:
  case ZiGraphicsBackend_D3D12:
  case ZiGraphicsBackend_WebGPU:
    zi_log_error("not supported yet");
    return;
  default:;
    return;
  }

  device.init();
}

void zi_graphics_terminate() {
  device.terminate();
}
