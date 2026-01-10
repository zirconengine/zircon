#include "zi_graphics.h"
#include "zi_log.h"

#ifdef ZI_VULKAN_ENABLED

#include <string.h>

#include "volk.h"
#include "zi_core.h"
#include "vulkan/vk_enum_string_helper.h"


void zi_platform_set_vulkan_loader(const PFN_vkGetInstanceProcAddr p_vulkan_loader);
const char** zi_platform_get_required_extensions(u32* count);
static VkBool32 vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
																			VkDebugUtilsMessageTypeFlagsEXT             messageType,
																			const VkDebugUtilsMessengerCallbackDataEXT* callbackDataExt,
																			void*                                       userData);
static ZiBool vulkan_query_layer_property(const char* layer);
static ZiBool vulkan_query_instance_extension(const char* required_extensions);
static ZiBool vulkan_query_instance_extensions(const char** required_extensions, u32 required_count);


static const char* validation_layer = "VK_LAYER_KHRONOS_validation";

static ZiBool      validation_layers_enabled = ZI_FALSE;
static ZiBool			 debug_utils_enabled = ZI_FALSE;

static VkInstance               instance = NULL;
static VkDebugUtilsMessengerEXT debug_utils_messenger_ext;

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

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

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

		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &validation_layer;
		createInfo.pNext = &debug_utils_messenger_create_info;
	} else {
		createInfo.enabledLayerCount = 0;
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
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
#endif

	createInfo.enabledExtensionCount = (u32)required_extensions.count;
	createInfo.ppEnabledExtensionNames = required_extensions.data;

	VkResult res = vkCreateInstance(&createInfo, NULL, &instance);

	if (res != VK_SUCCESS) {
		zi_log_error("Error on create vkCreateInstance %s", string_VkResult(res));
	}

	volkLoadInstance(instance);

	if (validation_layers_enabled) {
		vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_messenger_create_info, ZI_NULL, &debug_utils_messenger_ext);
	}

	zi_log_debug("vulkan initialized successfully");

	ConstStrArray_free(&required_extensions);
}

static void zi_vulkan_terminate() {

	if (validation_layers_enabled) {
		vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger_ext, ZI_NULL);
	}

	vkDestroyInstance(instance, NULL);
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