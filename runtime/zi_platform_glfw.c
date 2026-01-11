#include "zi_log.h"

#if ZI_DESKTOP


#ifdef ZI_VULKAN_ENABLED
#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#endif


#include <GLFW/glfw3.h>

#include "zi_app.h"
#include "zi_graphics.h"

void zi_app_init();
void zi_app_loop();
void zi_app_terminate();

void zi_app_set_app_running(u8 p_is_running);
void zi_bootstrap(ZiAppSettings* settings);


static GLFWwindow*       main_window;
static ZiSwapchainHandle swapchain_handle;


#ifdef ZI_VULKAN_ENABLED

static PFN_vkGetInstanceProcAddr vulkan_loader;
static PFN_vkVoidFunction zi_vulkan_loader(VkInstance instance, const char* procName) {
	return vulkan_loader(instance, procName);
}

#endif

i32 zi_platform_run(int argc, char** argv) {

#ifdef ZI_VULKAN_ENABLED
	glfwInitVulkanLoader(zi_vulkan_loader);
#endif


	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	ZiAppSettings settings = {0};
	zi_bootstrap(&settings);

	zi_app_init();

	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	main_window = glfwCreateWindow(800, 600, settings.title, NULL, NULL);

	ZiSwapchainDesc swapchain_desc;
	swapchain_desc.window_handle = main_window;
	swapchain_desc.format = ZiFormat_Undefined;
	swapchain_desc.vsync = ZI_TRUE;

	swapchain_handle = zi_swapchain_create(&swapchain_desc);

	while (zi_app_is_running()) {
		glfwPollEvents();

		if (glfwWindowShouldClose(main_window)) {
			zi_app_set_app_running(ZI_FALSE);
		}

		zi_app_loop();
	}

	zi_swapchain_destroy(swapchain_handle);
	glfwDestroyWindow(main_window);

	zi_app_terminate();

	glfwTerminate();

	return 0;
}

#ifdef ZI_VULKAN_ENABLED

void zi_platform_set_vulkan_loader(const PFN_vkGetInstanceProcAddr p_vulkan_loader) {
	vulkan_loader = p_vulkan_loader;
}

const char** zi_platform_get_required_extensions(u32* count) {
	return glfwGetRequiredInstanceExtensions(count);
}

ZiBool zi_get_physical_device_presentation_support(VkInstance instance, VkPhysicalDevice device, uint32_t queue_family) {
	return glfwGetPhysicalDevicePresentationSupport(instance, device, queue_family);
}

void zi_platform_create_surface(VkInstance instance, VoidPtr window_handle, VkSurfaceKHR* surface) {
	glfwCreateWindowSurface(instance, window_handle, ZI_NULL, surface);
}
#endif
#endif
