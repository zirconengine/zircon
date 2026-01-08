#include "zi_graphics.h"

#include "zi_log.h"
#include "zi_platform.h"

void zi_graphics_init_vulkan(ZiRenderDevice* device);
void zi_graphics_init_webgpu(ZiRenderDevice* device);

static ZiRenderDevice device = {};

void zi_graphics_init(ZiGraphicsBackend backend) {

	backend = zi_platform_get_graphics_backend(backend);

	switch (backend) {
		case ZiGraphicsBackend_Vulkan:
			zi_graphics_init_vulkan(&device);
			break;
		case ZiGraphicsBackend_WebGPU:
			zi_graphics_init_webgpu(&device);
			break;
		case ZiGraphicsBackend_Metal:
		case ZiGraphicsBackend_D3D12:
			zi_log_error("not supported yet");
			return;
		default: ;
			return;
	}

	device.init();
}

void zi_graphics_terminate() {
	device.terminate();
}
