
#include "zi_graphics.h"

#include <stdio.h>

#include "volk.h"
#include "vulkan/vk_enum_string_helper.h"

static VkInstance instance = NULL;

void zi_vulkan_init() {

  if (volkInitialize() != VK_SUCCESS) {
    // TODO
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
    printf("Error on create vkCreateInstance %s\n", string_VkResult(res));
  }

  printf("vulkan init\n");
}

void zi_vulkan_terminate() { printf("vulkan terminate\n"); }

void zi_graphics_init_vulkan(ZiRenderDevice *device) {
  device->init = zi_vulkan_init;
  device->terminate = zi_vulkan_terminate;
}
