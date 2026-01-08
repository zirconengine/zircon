#include "zi_graphics.h"

#include <stdio.h>

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
    printf("not supported yet\n");
    return;
  default:;
    return;
  }

  device.init();
}

void zi_graphics_terminate() {
  device.terminate();
}
