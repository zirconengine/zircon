#include "zi_graphics.h"
#include "zi_log.h"

#ifdef ZI_VULKAN_ENABLED


#include <string.h>

#include "volk.h"

#include "vk_mem_alloc.h"
#include "zi_core.h"
#include "vulkan/vk_enum_string_helper.h"


typedef struct ZiVulkanAdapter {
	VkPhysicalDevice device;
	u32              score;
	u32              graphics_family;
	u32              present_family;

	VkPhysicalDeviceProperties2                      device_properties;
	VkPhysicalDeviceRayQueryFeaturesKHR              device_ray_query_features_khr;
	VkPhysicalDeviceAccelerationStructureFeaturesKHR device_acceleration_structure_features_khr;
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR    device_ray_tracing_pipeline_features;
	VkPhysicalDeviceBufferDeviceAddressFeatures      buffer_device_address_features;
	VkPhysicalDeviceShaderDrawParametersFeatures     draw_parameters_features;
	VkPhysicalDeviceDescriptorIndexingFeatures       indexing_features;
	VkPhysicalDeviceMaintenance4FeaturesKHR          maintenance4_features;
	VkPhysicalDeviceMultiviewFeatures                multiview_features;
	VkPhysicalDeviceFeatures2                        device_features;
} ZiVulkanAdapter;

typedef struct ZiVulkanBaseInStructure {
	VkStructureType sType;
	VoidPtr         pNext;
} ZiVulkanBaseInStructure;

typedef struct ZiVulkanExtension {
	const char**           added_extensions;
	u32                    add_extension_count;
	VkExtensionProperties* available_extensions;
	u32                    available_extension_count;
} ZiVulkanExtension;


void         zi_platform_set_vulkan_loader(const PFN_vkGetInstanceProcAddr p_vulkan_loader);
const char** zi_platform_get_required_extensions(u32* count);
ZiBool       zi_get_physical_device_presentation_support(VkInstance instance, VkPhysicalDevice device, uint32_t queue_family);

static VkBool32 vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                      const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt,
                                      void*                                       userData);
static ZiBool vulkan_query_layer_property(const char* layer);
static ZiBool vulkan_query_instance_extension(const char* required_extensions);
static ZiBool vulkan_query_instance_extensions(const char** required_extensions, u32 required_count);
static void   vulkan_check_physical_device(ZiVulkanAdapter* adapter);
static ZiBool vulkan_add_if_present(ZiVulkanExtension* extensions, const char* extension, VoidPtr feature);
static void   vulkan_add_to_chain(VkPhysicalDeviceFeatures2* features, VoidPtr feature);


static const char* validation_layer = "VK_LAYER_KHRONOS_validation";

static ZiBool validation_layers_enabled = ZI_FALSE;
static ZiBool debug_utils_enabled = ZI_FALSE;

static VkInstance               instance = ZI_NULL;
static VkDebugUtilsMessengerEXT debug_utils_messenger_ext = ZI_NULL;
static ZiVulkanAdapter*         selected_adapter = ZI_NULL;
static VkDevice                 device = ZI_NULL;
static VmaAllocator             vma_allocator = ZI_NULL;
static VkDescriptorPool         descriptor_pool;
static VkFence                  in_flight_fences[ZI_FRAMES_IN_FLIGHT];

//global command buffers;
static VkCommandPool   command_pool;
static VkCommandBuffer command_buffers[ZI_FRAMES_IN_FLIGHT];

static VkQueue graphics_queue = ZI_NULL;
static VkQueue present_queue = ZI_NULL;

static ZiDeviceFeatures features;
static ZiBool           has_buffer_device_address = ZI_FALSE;

static ZiVulkanAdapter* adapters;
static u32              adapters_count;

static void zi_vulkan_init() {
	ZiBool enable_debug_layers = ZI_TRUE;

	if (volkInitialize() != VK_SUCCESS) {
		zi_log_error("error on call volkInitialize");
	}

	zi_platform_set_vulkan_loader(vkGetInstanceProcAddr);

	VkApplicationInfo applicationInfo = {0};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Zircon Engine";
	applicationInfo.applicationVersion = 0;
	applicationInfo.pEngineName = "Zircon Engine";
	applicationInfo.engineVersion = 0;
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instance_create_info = {0};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &applicationInfo;

	validation_layers_enabled = enable_debug_layers && vulkan_query_layer_property(validation_layer);

	VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

	if (validation_layers_enabled == ZI_TRUE) {
		debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		debug_utils_messenger_create_info.pfnUserCallback = &vulkan_debug_callback;

		instance_create_info.enabledLayerCount = 1;
		instance_create_info.ppEnabledLayerNames = &validation_layer;
		instance_create_info.pNext = &debug_utils_messenger_create_info;
	}
	else {
		instance_create_info.enabledLayerCount = 0;
	}

	ConstStrArray required_extensions = {};
	ConstStrArray_init(&required_extensions, 0);

	u32 count;

	const char** platform_extensions = zi_platform_get_required_extensions(&count);
	for (u32 i = 0; i < count; ++i) {
		ConstStrArray_push(&required_extensions, platform_extensions[i]);
	}

	debug_utils_enabled = enable_debug_layers && vulkan_query_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	if (debug_utils_enabled) {
		ConstStrArray_push(&required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

#if ZI_APPLE
	if (vulkan_query_instance_extension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
		ConstStrArray_push(&required_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
#endif

	instance_create_info.enabledExtensionCount = (u32)required_extensions.count;
	instance_create_info.ppEnabledExtensionNames = required_extensions.data;

	VkResult res = vkCreateInstance(&instance_create_info, NULL, &instance);

	ConstStrArray_free(&required_extensions);

	if (res != VK_SUCCESS) {
		zi_log_error("Error on create vkCreateInstance %s", string_VkResult(res));
	}

	volkLoadInstance(instance);

	if (validation_layers_enabled) {
		vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_messenger_create_info, ZI_NULL, &debug_utils_messenger_ext);
	}


	vkEnumeratePhysicalDevices(instance, &adapters_count, NULL);
	VkPhysicalDevice* devices = zi_mem_alloc(sizeof(VkPhysicalDevice) * adapters_count);
	vkEnumeratePhysicalDevices(instance, &adapters_count, devices);
	adapters = zi_mem_alloc(sizeof(ZiVulkanAdapter) * adapters_count);

	for (u32 i = 0; i < adapters_count; ++i) {
		ZiVulkanAdapter* adapter = &adapters[i];
		memset(adapter, 0, sizeof(ZiVulkanAdapter));
		adapter->device = devices[i];
		vulkan_check_physical_device(adapter);
	}

	zi_mem_free(devices);

	selected_adapter = 0;
	u32 score = 0;
	for (u32 i = 0; i < adapters_count; ++i) {
		if (adapters[i].score > score) {
			score = adapters[i].score;
			selected_adapter = &adapters[i];
		}
	}

	if (selected_adapter == ZI_NULL) {
		zi_log_debug("no device found for vulkan");
		return;
	}

	VkPhysicalDeviceFeatures2 device_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	ZiVulkanExtension extensions;
	memset(&extensions, 0, sizeof(ZiVulkanExtension));

	vkEnumerateDeviceExtensionProperties(selected_adapter->device, ZI_NULL, &extensions.available_extension_count, ZI_NULL);

	extensions.available_extensions = zi_mem_alloc(extensions.available_extension_count * sizeof(VkExtensionProperties));
	extensions.added_extensions = zi_mem_alloc(extensions.available_extension_count * sizeof(char*));

	vkEnumerateDeviceExtensionProperties(selected_adapter->device, ZI_NULL, &extensions.available_extension_count, extensions.available_extensions);

	device_features.features.samplerAnisotropy = selected_adapter->device_features.features.samplerAnisotropy;
	device_features.features.sampleRateShading = selected_adapter->device_features.features.sampleRateShading;
	device_features.features.depthClamp = ZI_TRUE;

	VkPhysicalDeviceMaintenance4Features maintenance4Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
	maintenance4Features.maintenance4 = ZI_TRUE;

	VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
	indexing_features.descriptorBindingPartiallyBound = selected_adapter->indexing_features.descriptorBindingPartiallyBound;
	indexing_features.runtimeDescriptorArray = selected_adapter->indexing_features.runtimeDescriptorArray;
	indexing_features.shaderSampledImageArrayNonUniformIndexing = selected_adapter->indexing_features.shaderSampledImageArrayNonUniformIndexing;
	indexing_features.shaderStorageBufferArrayNonUniformIndexing = selected_adapter->indexing_features.shaderStorageBufferArrayNonUniformIndexing;
	indexing_features.shaderUniformBufferArrayNonUniformIndexing = selected_adapter->indexing_features.shaderUniformBufferArrayNonUniformIndexing;
	indexing_features.descriptorBindingSampledImageUpdateAfterBind = selected_adapter->indexing_features.descriptorBindingSampledImageUpdateAfterBind;
	indexing_features.descriptorBindingStorageImageUpdateAfterBind = selected_adapter->indexing_features.descriptorBindingStorageImageUpdateAfterBind;
	indexing_features.descriptorBindingStorageBufferUpdateAfterBind = selected_adapter->indexing_features.descriptorBindingSampledImageUpdateAfterBind;
	indexing_features.descriptorBindingUniformBufferUpdateAfterBind = selected_adapter->indexing_features.descriptorBindingUniformBufferUpdateAfterBind;

	features.bindless_texture_supported = indexing_features.shaderSampledImageArrayNonUniformIndexing &&
		indexing_features.descriptorBindingPartiallyBound &&
		indexing_features.runtimeDescriptorArray &&
		indexing_features.descriptorBindingSampledImageUpdateAfterBind &&
		indexing_features.descriptorBindingStorageImageUpdateAfterBind;

	features.bindless_buffer_supported = indexing_features.shaderStorageBufferArrayNonUniformIndexing &&
		indexing_features.descriptorBindingPartiallyBound &&
		indexing_features.runtimeDescriptorArray &&
		indexing_features.descriptorBindingStorageBufferUpdateAfterBind &&
		indexing_features.descriptorBindingStorageImageUpdateAfterBind &&
		indexing_features.descriptorBindingUniformBufferUpdateAfterBind;


	features.multiview_enabled = selected_adapter->multiview_features.multiview;

	VkPhysicalDeviceRayQueryFeaturesKHR deviceRayQueryFeaturesKhr = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, ZI_NULL};
	deviceRayQueryFeaturesKhr.rayQuery = ZI_TRUE;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeaturesKhr = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, ZI_NULL};
	deviceAccelerationStructureFeaturesKhr.accelerationStructure = ZI_TRUE;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, ZI_NULL};
	deviceRayTracingPipelineFeatures.rayTracingPipeline = ZI_TRUE;

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, ZI_NULL};
	bufferDeviceAddressFeatures.bufferDeviceAddress = ZI_TRUE;

	VkPhysicalDeviceShaderDrawParametersFeatures drawParametersFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES, ZI_NULL};
	drawParametersFeatures.shaderDrawParameters = selected_adapter->draw_parameters_features.shaderDrawParameters;

	VkPhysicalDeviceMultiviewFeatures multiviewFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES, ZI_NULL};
	multiviewFeatures.multiview = ZI_TRUE;

	if (!vulkan_add_if_present(&extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME, 0)) {
		return;
	}

	if (!vulkan_add_if_present(&extensions, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, 0)) {
		return;
	}

	features.resolve_depth = vulkan_add_if_present(&extensions, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, 0);

	vulkan_add_if_present(&extensions, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_MAINTENANCE_4_EXTENSION_NAME, &maintenance4Features);

	has_buffer_device_address = vulkan_add_if_present(&extensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &bufferDeviceAddressFeatures);
	features.draw_indirect_count = vulkan_add_if_present(&extensions, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, 0);

	features.ray_tracing = vulkan_add_if_present(&extensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, 0) &&
		vulkan_add_if_present(&extensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, 0);

	vulkan_add_if_present(&extensions, VK_KHR_RAY_QUERY_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_SPIRV_1_4_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, 0);
	vulkan_add_if_present(&extensions, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, 0);

	if (features.bindless_texture_supported) {
		vulkan_add_to_chain(&device_features, &indexing_features);
	}

	if (features.ray_tracing) {
		vulkan_add_to_chain(&device_features, &deviceRayQueryFeaturesKhr);
		vulkan_add_to_chain(&device_features, &deviceAccelerationStructureFeaturesKhr);
		vulkan_add_to_chain(&device_features, &deviceRayTracingPipelineFeatures);
	}

	if (features.multiview_enabled) {
		vulkan_add_to_chain(&device_features, &multiviewFeatures);
	}

	if (drawParametersFeatures.shaderDrawParameters) {
		vulkan_add_to_chain(&device_features, &drawParametersFeatures);
	}

#ifdef SK_APPLE
	vulkan_add_if_present(&extensions, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, 0);
#endif

	float queue_priority = 1.0f;

	u32                     queue_create_info_count = 0;
	VkDeviceQueueCreateInfo queue_create_info[2] = {0};

	if (selected_adapter->graphics_family != selected_adapter->present_family) {
		queue_create_info_count = 2;

		queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[0].queueFamilyIndex = selected_adapter->graphics_family;
		queue_create_info[0].queueCount = 1;
		queue_create_info[0].pQueuePriorities = &queue_priority;

		queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[1].queueFamilyIndex = selected_adapter->present_family;
		queue_create_info[1].queueCount = 1;
		queue_create_info[1].pQueuePriorities = &queue_priority;
	}
	else {
		queue_create_info_count = 1;
		queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[0].queueFamilyIndex = selected_adapter->graphics_family;
		queue_create_info[0].queueCount = 1;
		queue_create_info[0].pQueuePriorities = &queue_priority;
	}

	if (selected_adapter->device_features.features.fillModeNonSolid) {
		device_features.features.fillModeNonSolid = VK_TRUE;
	}

	if (selected_adapter->device_features.features.wideLines) {
		device_features.features.wideLines = VK_TRUE;
	}

	VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0};
	device_create_info.pNext = &device_features;
	device_create_info.pQueueCreateInfos = queue_create_info;
	device_create_info.queueCreateInfoCount = queue_create_info_count;
	device_create_info.enabledExtensionCount = extensions.add_extension_count;
	device_create_info.ppEnabledExtensionNames = extensions.added_extensions;

	res = vkCreateDevice(selected_adapter->device, &device_create_info, ZI_NULL, &device);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create logical device for device %s, error %s",
		             selected_adapter->device_properties.properties.deviceName, string_VkResult(res));
		return;
	}

	zi_mem_free(extensions.available_extensions);
	zi_mem_free(extensions.added_extensions);

	vkGetDeviceQueue(device, selected_adapter->graphics_family, 0, &graphics_queue);
	vkGetDeviceQueue(device, selected_adapter->present_family, 0, &present_queue);

	VmaVulkanFunctions vma_vulkan_functions = {0};
	vma_vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vma_vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo vma_allocator_create_info = {0};
	vma_allocator_create_info.physicalDevice = selected_adapter->device;
	vma_allocator_create_info.device = device;
	vma_allocator_create_info.instance = instance;

	if (has_buffer_device_address) {
		vma_allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}

	vma_allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
	vmaCreateAllocator(&vma_allocator_create_info, &vma_allocator);

	VkDescriptorPoolSize sizes[] = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5000},
		{VK_DESCRIPTOR_TYPE_SAMPLER, 5000},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 5000},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5000},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5000}
	};

	VkDescriptorPoolCreateInfo pool_info = {0};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 6;
	pool_info.pPoolSizes = sizes;
	pool_info.maxSets = 5000;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	vkCreateDescriptorPool(device, &pool_info, ZI_NULL, &descriptor_pool);

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < ZI_FRAMES_IN_FLIGHT; ++i) {
		vkCreateFence(device, &fenceInfo, ZI_NULL, &in_flight_fences[i]);
	}

	VkCommandPoolCreateInfo command_pool_info = {0};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = selected_adapter->graphics_family;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(device, &command_pool_info, ZI_NULL, &command_pool);

	for (int i = 0; i < ZI_FRAMES_IN_FLIGHT; ++i) {
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool;
		alloc_info.commandBufferCount = 1;
		vkAllocateCommandBuffers(device, &alloc_info, &command_buffers[i]);
	}

	zi_log_info("Vulkan API %i.%i.%i Device: %s ",
	            VK_VERSION_MAJOR(selected_adapter->device_properties.properties.apiVersion),
	            VK_VERSION_MINOR(selected_adapter->device_properties.properties.apiVersion),
	            VK_VERSION_PATCH(selected_adapter->device_properties.properties.apiVersion),
	            selected_adapter->device_properties.properties.deviceName);
}

static void zi_vulkan_terminate() {
	zi_mem_free(adapters);

	vkDestroyCommandPool(device, command_pool, ZI_NULL);

	for (u32 i = 0; i < ZI_FRAMES_IN_FLIGHT; ++i) {
		vkDestroyFence(device, in_flight_fences[i], ZI_NULL);
	}

	vkDestroyDescriptorPool(device, descriptor_pool, ZI_NULL);

	vmaDestroyAllocator(vma_allocator);
	vkDestroyDevice(device, ZI_NULL);

	if (validation_layers_enabled) {
		vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger_ext, ZI_NULL);
	}

	vkDestroyInstance(instance, ZI_NULL);

	zi_log_debug("vulkan terminated successfully");
}

static void zi_vulkan_get_device_limits(ZiDeviceLimits* limits) {}

// Buffer
static ZiBufferHandle zi_vulkan_buffer_create(const ZiBufferDesc* desc) {
	return (ZiBufferHandle){0};
}

static void zi_vulkan_buffer_destroy(ZiBufferHandle handle) {}

static void zi_vulkan_buffer_write(ZiBufferHandle handle, u64 offset, const void* data, u64 size) {}

static void* zi_vulkan_buffer_map(ZiBufferHandle handle, u64 offset, u64 size) {
	return NULL;
}

static void zi_vulkan_buffer_unmap(ZiBufferHandle handle) {}

// Texture
static ZiTextureHandle zi_vulkan_texture_create(const ZiTextureDesc* desc) {
	return (ZiTextureHandle){0};
}

static void zi_vulkan_texture_destroy(ZiTextureHandle handle) {}

// Texture View
static ZiTextureViewHandle zi_vulkan_texture_view_create(const ZiTextureViewDesc* desc) {
	return (ZiTextureViewHandle){0};
}

static void zi_vulkan_texture_view_destroy(ZiTextureViewHandle handle) {}

// Sampler
static ZiSamplerHandle zi_vulkan_sampler_create(const ZiSamplerDesc* desc) {
	return (ZiSamplerHandle){0};
}

static void zi_vulkan_sampler_destroy(ZiSamplerHandle handle) {}

// Shader
static ZiShaderHandle zi_vulkan_shader_create(const ZiShaderDesc* desc) {
	return (ZiShaderHandle){0};
}

static void zi_vulkan_shader_destroy(ZiShaderHandle handle) {}

// Pipeline Layout
static ZiPipelineLayoutHandle zi_vulkan_pipeline_layout_create(const ZiPipelineLayoutDesc* desc) {
	return (ZiPipelineLayoutHandle){0};
}

static void zi_vulkan_pipeline_layout_destroy(ZiPipelineLayoutHandle handle) {}

// Graphics Pipeline
static ZiPipelineHandle zi_vulkan_graphics_pipeline_create(const ZiGraphicsPipelineDesc* desc) {
	return (ZiPipelineHandle){0};
}

static void zi_vulkan_graphics_pipeline_destroy(ZiPipelineHandle handle) {}

// Compute Pipeline
static ZiPipelineHandle zi_vulkan_compute_pipeline_create(const ZiComputePipelineDesc* desc) {
	return (ZiPipelineHandle){0};
}

static void zi_vulkan_compute_pipeline_destroy(ZiPipelineHandle handle) {}

// Bind Group Layout
static ZiBindGroupLayoutHandle zi_vulkan_bind_group_layout_create(const ZiBindGroupLayoutDesc* desc) {
	return (ZiBindGroupLayoutHandle){0};
}

static void zi_vulkan_bind_group_layout_destroy(ZiBindGroupLayoutHandle handle) {}

// Bind Group
static ZiBindGroupHandle zi_vulkan_bind_group_create(const ZiBindGroupDesc* desc) {
	return (ZiBindGroupHandle){0};
}

static void zi_vulkan_bind_group_destroy(ZiBindGroupHandle handle) {}

// Render Pass
static ZiRenderPassHandle zi_vulkan_render_pass_create(const ZiRenderPassDesc* desc) {
	return (ZiRenderPassHandle){0};
}

static void zi_vulkan_render_pass_destroy(ZiRenderPassHandle handle) {}

// Command Buffer
static ZiCommandBufferHandle zi_vulkan_command_buffer_create() {
	return (ZiCommandBufferHandle){0};
}

static void zi_vulkan_command_buffer_destroy(ZiCommandBufferHandle handle) {}

static void zi_vulkan_command_buffer_begin(ZiCommandBufferHandle handle) {}

static void zi_vulkan_command_buffer_end(ZiCommandBufferHandle handle) {}

static void zi_vulkan_command_buffer_submit(ZiCommandBufferHandle handle) {}

// Command Buffer - Render Pass
static void zi_vulkan_cmd_begin_render_pass(ZiCommandBufferHandle cmd, const ZiRenderPassDesc* desc) {}

static void zi_vulkan_cmd_end_render_pass(ZiCommandBufferHandle cmd) {}

// Command Buffer - State
static void zi_vulkan_cmd_set_pipeline(ZiCommandBufferHandle cmd, ZiPipelineHandle pipeline) {}

static void zi_vulkan_cmd_set_bind_group(ZiCommandBufferHandle cmd, u32 index, ZiBindGroupHandle bind_group) {}

static void zi_vulkan_cmd_set_vertex_buffer(ZiCommandBufferHandle cmd, u32 slot, ZiBufferHandle buffer, u64 offset) {}

static void zi_vulkan_cmd_set_index_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, ZiIndexFormat format) {}

static void zi_vulkan_cmd_push_constants(ZiCommandBufferHandle cmd, ZiShaderStage stages, u32 offset, u32 size, const void* data) {}

static void zi_vulkan_cmd_set_viewport(ZiCommandBufferHandle cmd, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth) {}

static void zi_vulkan_cmd_set_scissor(ZiCommandBufferHandle cmd, u32 x, u32 y, u32 width, u32 height) {}

static void zi_vulkan_cmd_set_blend_constant(ZiCommandBufferHandle cmd, f32 color[4]) {}

static void zi_vulkan_cmd_set_stencil_reference(ZiCommandBufferHandle cmd, u32 reference) {}

// Command Buffer - Draw
static void zi_vulkan_cmd_draw(ZiCommandBufferHandle cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {}

static void zi_vulkan_cmd_draw_indexed(ZiCommandBufferHandle cmd, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) {}

static void zi_vulkan_cmd_draw_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) {}

static void zi_vulkan_cmd_draw_indexed_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) {}

// Command Buffer - Compute
static void zi_vulkan_cmd_dispatch(ZiCommandBufferHandle cmd, u32 group_count_x, u32 group_count_y, u32 group_count_z) {}

static void zi_vulkan_cmd_dispatch_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset) {}

// Command Buffer - Copy
static void zi_vulkan_cmd_copy_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiBufferHandle dst, u64 dst_offset, u64 size) {}

static void zi_vulkan_cmd_copy_texture(ZiCommandBufferHandle cmd, ZiTextureHandle src, ZiTextureHandle dst) {}

static void zi_vulkan_cmd_copy_buffer_to_texture(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiTextureHandle dst, u32 mip_level, u32 array_layer) {}

static void zi_vulkan_cmd_copy_texture_to_buffer(ZiCommandBufferHandle cmd, ZiTextureHandle src, u32 mip_level, u32 array_layer, ZiBufferHandle dst, u64 dst_offset) {}

// Swapchain
static ZiSwapchainHandle zi_vulkan_swapchain_create(const ZiSwapchainDesc* desc) {
	return (ZiSwapchainHandle){0};
}

static void zi_vulkan_swapchain_destroy(ZiSwapchainHandle handle) {}

static void zi_vulkan_swapchain_resize(ZiSwapchainHandle handle, u32 width, u32 height) {}

static u32 zi_vulkan_swapchain_get_texture_count(ZiSwapchainHandle handle) {
	return 0;
}

static ZiTextureHandle zi_vulkan_swapchain_get_texture(ZiSwapchainHandle handle, u32 index) {
	return (ZiTextureHandle){0};
}

static void zi_vulkan_swapchain_present(ZiSwapchainHandle handle) {}

// Debug
static void zi_vulkan_set_object_name(void* handle, const char* name) {}

static void zi_vulkan_cmd_begin_debug_label(ZiCommandBufferHandle cmd, const char* label) {}

static void zi_vulkan_cmd_end_debug_label(ZiCommandBufferHandle cmd) {}

//helper impls
static VkBool32 vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                      const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt,
                                      void*                                       userData) {
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			zi_log(ZiLogLevel_Trace, callbackDataExt->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			zi_log(ZiLogLevel_Info, callbackDataExt->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			zi_log(ZiLogLevel_Warn, callbackDataExt->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			zi_log(ZiLogLevel_Error, callbackDataExt->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
			break;
	}

	return VK_FALSE;
}

static ZiBool vulkan_query_layer_property(const char* layer) {
	ZiBool ret = ZI_FALSE;

	u32 count = 0;
	vkEnumerateInstanceLayerProperties(&count, ZI_NULL);
	VkLayerProperties* props = zi_mem_alloc(count * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&count, props);

	for (u32 i = 0; i < count; ++i) {
		if (strcmp(layer, props[i].layerName) == 0) {
			ret = ZI_TRUE;
			break;
		}
	}

	zi_mem_free(props);

	return ret;
}

static ZiBool vulkan_query_instance_extension(const char* required_extensions) {
	return vulkan_query_instance_extensions(&required_extensions, 1);
}

static ZiBool vulkan_query_instance_extensions(const char** required_extensions, u32 required_count) {
	u32 count = 0;
	vkEnumerateInstanceExtensionProperties(ZI_NULL, &count, ZI_NULL);
	VkExtensionProperties* props = zi_mem_alloc(count * sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(ZI_NULL, &count, props);

	ZiBool ret = ZI_TRUE;

	for (u32 i = 0; i < required_count; ++i) {
		ZiBool found = ZI_FALSE;
		for (u32 j = 0; j < count; ++j) {
			if (strcmp(required_extensions[i], props[j].extensionName) == 0) {
				found = ZI_TRUE;
				break;
			}
		}

		if (found == ZI_FALSE) {
			ret = ZI_FALSE;
			break;
		}
	}

	zi_mem_free(props);

	return ret;
}

static void vulkan_check_physical_device(ZiVulkanAdapter* adapter) {
	adapter->graphics_family = U32_MAX;
	adapter->present_family = U32_MAX;

	adapter->device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	adapter->device_ray_query_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

	adapter->device_acceleration_structure_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	adapter->device_acceleration_structure_features_khr.pNext = &adapter->device_ray_query_features_khr;

	adapter->device_ray_tracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	adapter->device_ray_tracing_pipeline_features.pNext = &adapter->device_acceleration_structure_features_khr;

	adapter->buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	adapter->buffer_device_address_features.pNext = &adapter->device_ray_tracing_pipeline_features;

	adapter->draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	adapter->draw_parameters_features.pNext = &adapter->buffer_device_address_features;

	adapter->indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	adapter->indexing_features.pNext = &adapter->draw_parameters_features;

	adapter->maintenance4_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR;
	adapter->maintenance4_features.pNext = &adapter->indexing_features;

	adapter->multiview_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
	adapter->multiview_features.pNext = &adapter->maintenance4_features;

	adapter->device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	adapter->device_features.pNext = &adapter->multiview_features;

	vkGetPhysicalDeviceProperties2(adapter->device, &adapter->device_properties);
	vkGetPhysicalDeviceFeatures2(adapter->device, &adapter->device_features);


	adapter->score += adapter->device_properties.properties.limits.maxImageDimension2D / 1024;

	if (adapter->device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		adapter->score += 1000;
	}
	else if (adapter->device_properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
		adapter->score += 500;
	}

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(adapter->device, &queue_family_count, NULL);

	VkQueueFamilyProperties* queue_families = zi_mem_alloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(adapter->device, &queue_family_count, queue_families);

	ZiBool has_graphics_queue = ZI_FALSE;
	ZiBool has_compute_queue = ZI_FALSE;
	ZiBool has_transfer_queue = ZI_FALSE;
	ZiBool has_present_queue = ZI_FALSE;

	for (u32 i = 0; i < queue_family_count; ++i) {
		VkQueueFamilyProperties* queue_family_properties = &queue_families[i];

		ZiBool has_present_family = zi_get_physical_device_presentation_support(instance, adapter->device, i);
		ZiBool has_graphics_family = queue_family_properties->queueFlags & VK_QUEUE_GRAPHICS_BIT;

		if (has_graphics_family && adapter->graphics_family == U32_MAX) {
			adapter->graphics_family = i;
		}

		if (has_present_family && adapter->present_family == U32_MAX) {
			adapter->present_family = i;
		}

		//score device by queue
		if (has_present_family) {
			has_present_queue = ZI_TRUE;
		}

		if (has_graphics_family) {
			has_graphics_queue = ZI_TRUE;
		}
		if (queue_family_properties->queueFlags & VK_QUEUE_COMPUTE_BIT) {
			has_compute_queue = ZI_TRUE;
		}
		if (queue_family_properties->queueFlags & VK_QUEUE_TRANSFER_BIT) {
			has_transfer_queue = ZI_TRUE;
		}
	}

	if (has_compute_queue) {
		adapter->score += 100;
	}
	if (has_transfer_queue) {
		adapter->score += 100;
	}

	if (!has_graphics_queue || !has_present_queue) {
		adapter->score = 0;
	}

	zi_mem_free(queue_families);
}

static ZiBool vulkan_add_if_present(ZiVulkanExtension* extensions, const char* extension, VoidPtr feature) {
	//check for duplicates
	for (u32 i = 0; i < extensions->add_extension_count; ++i) {
		if (strcmp(extensions->added_extensions[i], extension) == 0) {
			return ZI_TRUE;
		}
	}


	for (u32 i = 0; i < extensions->available_extension_count; ++i) {
		if (strcmp(extensions->available_extensions[i].extensionName, extension) == 0) {
			extensions->added_extensions[extensions->add_extension_count] = extension;
			extensions->add_extension_count++;
			return ZI_TRUE;
		}
	}


	return ZI_FALSE;
}

static void vulkan_add_to_chain(VkPhysicalDeviceFeatures2* device_features, VoidPtr feature) {
	if (feature != ZI_NULL) {
		ZiVulkanBaseInStructure* to_add = feature;
		to_add->pNext = device_features->pNext;
		device_features->pNext = to_add;
	}
}

void zi_graphics_init_vulkan(ZiRenderDevice* device) {
	device->init = zi_vulkan_init;
	device->terminate = zi_vulkan_terminate;
	device->get_device_limits = zi_vulkan_get_device_limits;

	// Buffer
	device->buffer_create = zi_vulkan_buffer_create;
	device->buffer_destroy = zi_vulkan_buffer_destroy;
	device->buffer_write = zi_vulkan_buffer_write;
	device->buffer_map = zi_vulkan_buffer_map;
	device->buffer_unmap = zi_vulkan_buffer_unmap;

	// Texture
	device->texture_create = zi_vulkan_texture_create;
	device->texture_destroy = zi_vulkan_texture_destroy;

	// Texture View
	device->texture_view_create = zi_vulkan_texture_view_create;
	device->texture_view_destroy = zi_vulkan_texture_view_destroy;

	// Sampler
	device->sampler_create = zi_vulkan_sampler_create;
	device->sampler_destroy = zi_vulkan_sampler_destroy;

	// Shader
	device->shader_create = zi_vulkan_shader_create;
	device->shader_destroy = zi_vulkan_shader_destroy;

	// Pipeline Layout
	device->pipeline_layout_create = zi_vulkan_pipeline_layout_create;
	device->pipeline_layout_destroy = zi_vulkan_pipeline_layout_destroy;

	// Graphics Pipeline
	device->graphics_pipeline_create = zi_vulkan_graphics_pipeline_create;
	device->graphics_pipeline_destroy = zi_vulkan_graphics_pipeline_destroy;

	// Compute Pipeline
	device->compute_pipeline_create = zi_vulkan_compute_pipeline_create;
	device->compute_pipeline_destroy = zi_vulkan_compute_pipeline_destroy;

	// Bind Group Layout
	device->bind_group_layout_create = zi_vulkan_bind_group_layout_create;
	device->bind_group_layout_destroy = zi_vulkan_bind_group_layout_destroy;

	// Bind Group
	device->bind_group_create = zi_vulkan_bind_group_create;
	device->bind_group_destroy = zi_vulkan_bind_group_destroy;

	// Render Pass
	device->render_pass_create = zi_vulkan_render_pass_create;
	device->render_pass_destroy = zi_vulkan_render_pass_destroy;

	// Command Buffer
	device->command_buffer_create = zi_vulkan_command_buffer_create;
	device->command_buffer_destroy = zi_vulkan_command_buffer_destroy;
	device->command_buffer_begin = zi_vulkan_command_buffer_begin;
	device->command_buffer_end = zi_vulkan_command_buffer_end;
	device->command_buffer_submit = zi_vulkan_command_buffer_submit;

	// Command Buffer - Render Pass
	device->cmd_begin_render_pass = zi_vulkan_cmd_begin_render_pass;
	device->cmd_end_render_pass = zi_vulkan_cmd_end_render_pass;

	// Command Buffer - State
	device->cmd_set_pipeline = zi_vulkan_cmd_set_pipeline;
	device->cmd_set_bind_group = zi_vulkan_cmd_set_bind_group;
	device->cmd_set_vertex_buffer = zi_vulkan_cmd_set_vertex_buffer;
	device->cmd_set_index_buffer = zi_vulkan_cmd_set_index_buffer;
	device->cmd_push_constants = zi_vulkan_cmd_push_constants;
	device->cmd_set_viewport = zi_vulkan_cmd_set_viewport;
	device->cmd_set_scissor = zi_vulkan_cmd_set_scissor;
	device->cmd_set_blend_constant = zi_vulkan_cmd_set_blend_constant;
	device->cmd_set_stencil_reference = zi_vulkan_cmd_set_stencil_reference;

	// Command Buffer - Draw
	device->cmd_draw = zi_vulkan_cmd_draw;
	device->cmd_draw_indexed = zi_vulkan_cmd_draw_indexed;
	device->cmd_draw_indirect = zi_vulkan_cmd_draw_indirect;
	device->cmd_draw_indexed_indirect = zi_vulkan_cmd_draw_indexed_indirect;

	// Command Buffer - Compute
	device->cmd_dispatch = zi_vulkan_cmd_dispatch;
	device->cmd_dispatch_indirect = zi_vulkan_cmd_dispatch_indirect;

	// Command Buffer - Copy
	device->cmd_copy_buffer = zi_vulkan_cmd_copy_buffer;
	device->cmd_copy_texture = zi_vulkan_cmd_copy_texture;
	device->cmd_copy_buffer_to_texture = zi_vulkan_cmd_copy_buffer_to_texture;
	device->cmd_copy_texture_to_buffer = zi_vulkan_cmd_copy_texture_to_buffer;

	// Swapchain
	device->swapchain_create = zi_vulkan_swapchain_create;
	device->swapchain_destroy = zi_vulkan_swapchain_destroy;
	device->swapchain_resize = zi_vulkan_swapchain_resize;
	device->swapchain_get_texture_count = zi_vulkan_swapchain_get_texture_count;
	device->swapchain_get_texture = zi_vulkan_swapchain_get_texture;
	device->swapchain_present = zi_vulkan_swapchain_present;

	// Debug
	device->set_object_name = zi_vulkan_set_object_name;
	device->cmd_begin_debug_label = zi_vulkan_cmd_begin_debug_label;
	device->cmd_end_debug_label = zi_vulkan_cmd_end_debug_label;
}
#else
void zi_graphics_init_vulkan(ZiRenderDevice* device) {
	zi_log_error("vulkan not available on this platform");
}
#endif
