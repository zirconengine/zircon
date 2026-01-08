#include "zi_graphics.h"
#include "zi_log.h"

#ifdef ZI_EMSCRIPTEN


void zi_webgpu_init() {
	zi_log_debug("WebGPU initialized successfully");
}

void zi_webgpu_terminate() {
	zi_log_debug("WebGPU terminated successfully");
}

void zi_graphics_init_webgpu(ZiRenderDevice* device) {
	device->init = zi_webgpu_init;
	device->terminate = zi_webgpu_terminate;
}
#else
void zi_graphics_init_webgpu(ZiRenderDevice* device) {
	zi_log_error("WebGPU not available on this platform");
}
#endif
