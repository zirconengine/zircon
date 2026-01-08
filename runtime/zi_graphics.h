#pragma once
#include "zi_common.h"

enum ZiGraphicsBackend_ {
	ZiGraphicsBackend_Vulkan = 1,
	ZiGraphicsBackend_Metal  = 2,
	ZiGraphicsBackend_D3D12  = 3,
	ZiGraphicsBackend_WebGPU = 4
};

typedef u32 ZiGraphicsBackend;


typedef struct ZiRenderDevice {
	void (*init)();
	void (*terminate)();
} ZiRenderDevice;

void zi_graphics_init(ZiGraphicsBackend backend);
void zi_graphics_terminate();
