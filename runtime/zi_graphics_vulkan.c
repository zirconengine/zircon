#include "zi_graphics.h"

#include "volk.h"
#include "vulkan/vk_enum_string_helper.h"
#include "zi_log.h"

static VkInstance instance = NULL;

void zi_vulkan_init() {
	if (volkInitialize() != VK_SUCCESS) {
		zi_log_error("error on call volkInitialize");
	}

	VkApplicationInfo applicationInfo = {0};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Zircon Engine";
	applicationInfo.applicationVersion = 0;
	applicationInfo.pEngineName = "Zircon Engine";
	applicationInfo.engineVersion = 0;
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	VkResult res = vkCreateInstance(&createInfo, NULL, &instance);

	if (res != VK_SUCCESS) {
		zi_log_error("Error on create vkCreateInstance %s", string_VkResult(res));
	}

	volkLoadInstance(instance);

	zi_log_debug("vulkan initialized successfully");
}

void zi_vulkan_terminate() {
	vkDestroyInstance(instance, NULL);
	zi_log_debug("vulkan terminated successfully");
}

void zi_graphics_init_vulkan(ZiRenderDevice* device) {
	device->init = zi_vulkan_init;
	device->terminate = zi_vulkan_terminate;
}
