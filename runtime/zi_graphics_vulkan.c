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

typedef struct ZiVulkanBuffer {
	VkBuffer      buffer;
	VmaAllocation allocation;
	u64           size;
	ZiBufferUsage usage;
	ZiMemoryUsage memory;
} ZiVulkanBuffer;

typedef struct ZiVulkanTexture {
	VkImage           image;
	VmaAllocation     allocation;
	VkFormat          format;
	u32               width;
	u32               height;
	u32               depth;
	u32               mip_levels;
	u32               array_layers;
	u32               sample_count;
	ZiTextureDimension dimension;
	ZiTextureUsage    usage;
	ZiBool            is_swapchain_image;
} ZiVulkanTexture;

typedef struct ZiVulkanTextureView {
	VkImageView       view;
	ZiVulkanTexture*  texture;
} ZiVulkanTextureView;

typedef struct ZiVulkanSampler {
	VkSampler sampler;
} ZiVulkanSampler;

typedef struct ZiVulkanShader {
	VkShaderModule module;
	ZiShaderStage  stage;
	const char*    entry_point;
} ZiVulkanShader;

typedef struct ZiVulkanBindGroupLayout {
	VkDescriptorSetLayout layout;
	u32                   binding_count;
} ZiVulkanBindGroupLayout;

typedef struct ZiVulkanPipelineLayout {
	VkPipelineLayout layout;
	u32              push_constant_size;
} ZiVulkanPipelineLayout;

typedef struct ZiVulkanPipeline {
	VkPipeline       pipeline;
	VkPipelineLayout layout;
	VkPipelineBindPoint bind_point;
} ZiVulkanPipeline;

typedef struct ZiVulkanBindGroup {
	VkDescriptorSet set;
} ZiVulkanBindGroup;

typedef struct ZiVulkanRenderPass {
	VkRenderPass render_pass;
	u32          color_attachment_count;
	ZiBool       has_depth_attachment;
} ZiVulkanRenderPass;

typedef struct ZiVulkanFramebuffer {
	VkFramebuffer framebuffer;
	u32           width;
	u32           height;
	u32           layers;
} ZiVulkanFramebuffer;

typedef struct ZiVulkanCommandBuffer {
	VkCommandBuffer cmd;
	VkCommandPool   pool;
	VkFence         fence;
	ZiBool          is_recording;
} ZiVulkanCommandBuffer;

typedef struct ZiVulkanSwapchain {
	VkSurfaceKHR     surface;
	VkSwapchainKHR   swapchain;
	VkFormat         format;
	VkPresentModeKHR present_mode;
	u32              width;
	u32              height;
	u32              image_count;
	VkImage*         images;
	ZiVulkanTexture* textures;
	VkImageView*     image_views;
	VkSemaphore      image_available_semaphores[ZI_FRAMES_IN_FLIGHT];
	VkSemaphore      render_finished_semaphores[ZI_FRAMES_IN_FLIGHT];
	u32              current_image_index;
	u32              current_frame;
} ZiVulkanSwapchain;


void         zi_platform_set_vulkan_loader(const PFN_vkGetInstanceProcAddr p_vulkan_loader);
const char** zi_platform_get_required_extensions(u32* count);
ZiBool       zi_get_physical_device_presentation_support(VkInstance instance, VkPhysicalDevice device, uint32_t queue_family);
void         zi_platform_create_surface(VkInstance instance, VoidPtr window_handle, VkSurfaceKHR* surface);

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

// Format conversion helpers
static VkFormat zi_format_to_vk(ZiFormat format) {
	switch (format) {
		case ZiFormat_Undefined: return VK_FORMAT_UNDEFINED;
		case ZiFormat_R8_Unorm: return VK_FORMAT_R8_UNORM;
		case ZiFormat_R8_Snorm: return VK_FORMAT_R8_SNORM;
		case ZiFormat_R8_Uint: return VK_FORMAT_R8_UINT;
		case ZiFormat_R8_Sint: return VK_FORMAT_R8_SINT;
		case ZiFormat_R16_Unorm: return VK_FORMAT_R16_UNORM;
		case ZiFormat_R16_Snorm: return VK_FORMAT_R16_SNORM;
		case ZiFormat_R16_Uint: return VK_FORMAT_R16_UINT;
		case ZiFormat_R16_Sint: return VK_FORMAT_R16_SINT;
		case ZiFormat_R16_Float: return VK_FORMAT_R16_SFLOAT;
		case ZiFormat_RG8_Unorm: return VK_FORMAT_R8G8_UNORM;
		case ZiFormat_RG8_Snorm: return VK_FORMAT_R8G8_SNORM;
		case ZiFormat_RG8_Uint: return VK_FORMAT_R8G8_UINT;
		case ZiFormat_RG8_Sint: return VK_FORMAT_R8G8_SINT;
		case ZiFormat_R32_Uint: return VK_FORMAT_R32_UINT;
		case ZiFormat_R32_Sint: return VK_FORMAT_R32_SINT;
		case ZiFormat_R32_Float: return VK_FORMAT_R32_SFLOAT;
		case ZiFormat_RG16_Unorm: return VK_FORMAT_R16G16_UNORM;
		case ZiFormat_RG16_Snorm: return VK_FORMAT_R16G16_SNORM;
		case ZiFormat_RG16_Uint: return VK_FORMAT_R16G16_UINT;
		case ZiFormat_RG16_Sint: return VK_FORMAT_R16G16_SINT;
		case ZiFormat_RG16_Float: return VK_FORMAT_R16G16_SFLOAT;
		case ZiFormat_RGBA8_Unorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ZiFormat_RGBA8_Snorm: return VK_FORMAT_R8G8B8A8_SNORM;
		case ZiFormat_RGBA8_Uint: return VK_FORMAT_R8G8B8A8_UINT;
		case ZiFormat_RGBA8_Sint: return VK_FORMAT_R8G8B8A8_SINT;
		case ZiFormat_RGBA8_Srgb: return VK_FORMAT_R8G8B8A8_SRGB;
		case ZiFormat_BGRA8_Unorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ZiFormat_BGRA8_Srgb: return VK_FORMAT_B8G8R8A8_SRGB;
		case ZiFormat_RG32_Uint: return VK_FORMAT_R32G32_UINT;
		case ZiFormat_RG32_Sint: return VK_FORMAT_R32G32_SINT;
		case ZiFormat_RG32_Float: return VK_FORMAT_R32G32_SFLOAT;
		case ZiFormat_RGBA16_Unorm: return VK_FORMAT_R16G16B16A16_UNORM;
		case ZiFormat_RGBA16_Snorm: return VK_FORMAT_R16G16B16A16_SNORM;
		case ZiFormat_RGBA16_Uint: return VK_FORMAT_R16G16B16A16_UINT;
		case ZiFormat_RGBA16_Sint: return VK_FORMAT_R16G16B16A16_SINT;
		case ZiFormat_RGBA16_Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case ZiFormat_RGBA32_Uint: return VK_FORMAT_R32G32B32A32_UINT;
		case ZiFormat_RGBA32_Sint: return VK_FORMAT_R32G32B32A32_SINT;
		case ZiFormat_RGBA32_Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ZiFormat_RGB10A2_Unorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case ZiFormat_RGB10A2_Uint: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case ZiFormat_RG11B10_Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case ZiFormat_RGB9E5_Float: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		case ZiFormat_D16_Unorm: return VK_FORMAT_D16_UNORM;
		case ZiFormat_D24_Unorm_S8_Uint: return VK_FORMAT_D24_UNORM_S8_UINT;
		case ZiFormat_D32_Float: return VK_FORMAT_D32_SFLOAT;
		case ZiFormat_D32_Float_S8_Uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case ZiFormat_S8_Uint: return VK_FORMAT_S8_UINT;
		case ZiFormat_BC1_Unorm: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case ZiFormat_BC1_Srgb: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case ZiFormat_BC2_Unorm: return VK_FORMAT_BC2_UNORM_BLOCK;
		case ZiFormat_BC2_Srgb: return VK_FORMAT_BC2_SRGB_BLOCK;
		case ZiFormat_BC3_Unorm: return VK_FORMAT_BC3_UNORM_BLOCK;
		case ZiFormat_BC3_Srgb: return VK_FORMAT_BC3_SRGB_BLOCK;
		case ZiFormat_BC4_Unorm: return VK_FORMAT_BC4_UNORM_BLOCK;
		case ZiFormat_BC4_Snorm: return VK_FORMAT_BC4_SNORM_BLOCK;
		case ZiFormat_BC5_Unorm: return VK_FORMAT_BC5_UNORM_BLOCK;
		case ZiFormat_BC5_Snorm: return VK_FORMAT_BC5_SNORM_BLOCK;
		case ZiFormat_BC6H_UFloat: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case ZiFormat_BC6H_SFloat: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case ZiFormat_BC7_Unorm: return VK_FORMAT_BC7_UNORM_BLOCK;
		case ZiFormat_BC7_Srgb: return VK_FORMAT_BC7_SRGB_BLOCK;
		default: return VK_FORMAT_UNDEFINED;
	}
}

static VkBufferUsageFlags zi_buffer_usage_to_vk(ZiBufferUsage usage) {
	VkBufferUsageFlags flags = 0;
	if (usage & ZiBufferUsage_Vertex) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (usage & ZiBufferUsage_Index) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (usage & ZiBufferUsage_Uniform) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (usage & ZiBufferUsage_Storage) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (usage & ZiBufferUsage_CopySrc) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (usage & ZiBufferUsage_CopyDst) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	return flags;
}

static VmaMemoryUsage zi_memory_usage_to_vma(ZiMemoryUsage usage) {
	switch (usage) {
		case ZiMemoryUsage_GpuOnly: return VMA_MEMORY_USAGE_GPU_ONLY;
		case ZiMemoryUsage_CpuToGpu: return VMA_MEMORY_USAGE_CPU_TO_GPU;
		case ZiMemoryUsage_GpuToCpu: return VMA_MEMORY_USAGE_GPU_TO_CPU;
		default: return VMA_MEMORY_USAGE_AUTO;
	}
}

static VkImageUsageFlags zi_texture_usage_to_vk(ZiTextureUsage usage) {
	VkImageUsageFlags flags = 0;
	if (usage & ZiTextureUsage_Sampled) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usage & ZiTextureUsage_Storage) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (usage & ZiTextureUsage_RenderTarget) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (usage & ZiTextureUsage_DepthStencil) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (usage & ZiTextureUsage_CopySrc) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (usage & ZiTextureUsage_CopyDst) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	return flags;
}

static VkImageType zi_dimension_to_vk_image_type(ZiTextureDimension dim) {
	switch (dim) {
		case ZiTextureDimension_1D: return VK_IMAGE_TYPE_1D;
		case ZiTextureDimension_2D: return VK_IMAGE_TYPE_2D;
		case ZiTextureDimension_3D: return VK_IMAGE_TYPE_3D;
		default: return VK_IMAGE_TYPE_2D;
	}
}

static VkImageViewType zi_view_dimension_to_vk(ZiTextureViewDimension dim) {
	switch (dim) {
		case ZiTextureViewDimension_1D: return VK_IMAGE_VIEW_TYPE_1D;
		case ZiTextureViewDimension_2D: return VK_IMAGE_VIEW_TYPE_2D;
		case ZiTextureViewDimension_2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case ZiTextureViewDimension_Cube: return VK_IMAGE_VIEW_TYPE_CUBE;
		case ZiTextureViewDimension_CubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		case ZiTextureViewDimension_3D: return VK_IMAGE_VIEW_TYPE_3D;
		default: return VK_IMAGE_VIEW_TYPE_2D;
	}
}

static VkImageAspectFlags zi_aspect_to_vk(ZiTextureAspect aspect, VkFormat format) {
	switch (aspect) {
		case ZiTextureAspect_Color: return VK_IMAGE_ASPECT_COLOR_BIT;
		case ZiTextureAspect_Depth: return VK_IMAGE_ASPECT_DEPTH_BIT;
		case ZiTextureAspect_Stencil: return VK_IMAGE_ASPECT_STENCIL_BIT;
		case ZiTextureAspect_All:
		default:
			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			} else if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT) {
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			} else if (format == VK_FORMAT_S8_UINT) {
				return VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

static VkFilter zi_filter_to_vk(ZiFilterMode filter) {
	switch (filter) {
		case ZiFilterMode_Nearest: return VK_FILTER_NEAREST;
		case ZiFilterMode_Linear: return VK_FILTER_LINEAR;
		default: return VK_FILTER_LINEAR;
	}
}

static VkSamplerMipmapMode zi_mipmap_filter_to_vk(ZiFilterMode filter) {
	switch (filter) {
		case ZiFilterMode_Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case ZiFilterMode_Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

static VkSamplerAddressMode zi_address_mode_to_vk(ZiAddressMode mode) {
	switch (mode) {
		case ZiAddressMode_Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case ZiAddressMode_MirrorRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case ZiAddressMode_ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case ZiAddressMode_ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static VkCompareOp zi_compare_to_vk(ZiCompareFunc compare) {
	switch (compare) {
		case ZiCompareFunc_Never: return VK_COMPARE_OP_NEVER;
		case ZiCompareFunc_Less: return VK_COMPARE_OP_LESS;
		case ZiCompareFunc_Equal: return VK_COMPARE_OP_EQUAL;
		case ZiCompareFunc_LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case ZiCompareFunc_Greater: return VK_COMPARE_OP_GREATER;
		case ZiCompareFunc_NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
		case ZiCompareFunc_GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case ZiCompareFunc_Always: return VK_COMPARE_OP_ALWAYS;
		default: return VK_COMPARE_OP_ALWAYS;
	}
}

static VkShaderStageFlagBits zi_shader_stage_to_vk(ZiShaderStage stage) {
	VkShaderStageFlags flags = 0;
	if (stage & ZiShaderStage_Vertex) flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (stage & ZiShaderStage_Fragment) flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (stage & ZiShaderStage_Compute) flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	return flags;
}

static VkDescriptorType zi_binding_type_to_vk(ZiBindingType type) {
	switch (type) {
		case ZiBindingType_UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case ZiBindingType_StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case ZiBindingType_Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case ZiBindingType_SampledTexture: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case ZiBindingType_StorageTexture: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		default: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}

static VkPrimitiveTopology zi_topology_to_vk(ZiPrimitiveTopology topology) {
	switch (topology) {
		case ZiPrimitiveTopology_PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case ZiPrimitiveTopology_LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case ZiPrimitiveTopology_LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case ZiPrimitiveTopology_TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case ZiPrimitiveTopology_TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

static VkCullModeFlags zi_cull_mode_to_vk(ZiCullMode mode) {
	switch (mode) {
		case ZiCullMode_None: return VK_CULL_MODE_NONE;
		case ZiCullMode_Front: return VK_CULL_MODE_FRONT_BIT;
		case ZiCullMode_Back: return VK_CULL_MODE_BACK_BIT;
		default: return VK_CULL_MODE_NONE;
	}
}

static VkFrontFace zi_front_face_to_vk(ZiFrontFace face) {
	switch (face) {
		case ZiFrontFace_CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		case ZiFrontFace_CW: return VK_FRONT_FACE_CLOCKWISE;
		default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

static VkBlendFactor zi_blend_factor_to_vk(ZiBlendFactor factor) {
	switch (factor) {
		case ZiBlendFactor_Zero: return VK_BLEND_FACTOR_ZERO;
		case ZiBlendFactor_One: return VK_BLEND_FACTOR_ONE;
		case ZiBlendFactor_SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
		case ZiBlendFactor_OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case ZiBlendFactor_SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
		case ZiBlendFactor_OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case ZiBlendFactor_DstColor: return VK_BLEND_FACTOR_DST_COLOR;
		case ZiBlendFactor_OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case ZiBlendFactor_DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
		case ZiBlendFactor_OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		default: return VK_BLEND_FACTOR_ONE;
	}
}

static VkBlendOp zi_blend_op_to_vk(ZiBlendOp op) {
	switch (op) {
		case ZiBlendOp_Add: return VK_BLEND_OP_ADD;
		case ZiBlendOp_Subtract: return VK_BLEND_OP_SUBTRACT;
		case ZiBlendOp_ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case ZiBlendOp_Min: return VK_BLEND_OP_MIN;
		case ZiBlendOp_Max: return VK_BLEND_OP_MAX;
		default: return VK_BLEND_OP_ADD;
	}
}

static VkAttachmentLoadOp zi_load_op_to_vk(ZiLoadOp op) {
	switch (op) {
		case ZiLoadOp_Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case ZiLoadOp_Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case ZiLoadOp_DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

static VkAttachmentStoreOp zi_store_op_to_vk(ZiStoreOp op) {
	switch (op) {
		case ZiStoreOp_Store: return VK_ATTACHMENT_STORE_OP_STORE;
		case ZiStoreOp_DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		default: return VK_ATTACHMENT_STORE_OP_STORE;
	}
}

static VkIndexType zi_index_format_to_vk(ZiIndexFormat format) {
	switch (format) {
		case ZiIndexFormat_Uint16: return VK_INDEX_TYPE_UINT16;
		case ZiIndexFormat_Uint32: return VK_INDEX_TYPE_UINT32;
		default: return VK_INDEX_TYPE_UINT32;
	}
}

static VkSampleCountFlagBits zi_sample_count_to_vk(u32 count) {
	switch (count) {
		case 1: return VK_SAMPLE_COUNT_1_BIT;
		case 2: return VK_SAMPLE_COUNT_2_BIT;
		case 4: return VK_SAMPLE_COUNT_4_BIT;
		case 8: return VK_SAMPLE_COUNT_8_BIT;
		case 16: return VK_SAMPLE_COUNT_16_BIT;
		case 32: return VK_SAMPLE_COUNT_32_BIT;
		case 64: return VK_SAMPLE_COUNT_64_BIT;
		default: return VK_SAMPLE_COUNT_1_BIT;
	}
}

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

static void zi_vulkan_get_device_limits(ZiDeviceLimits* limits) {
	if (!limits || !selected_adapter) return;

	VkPhysicalDeviceLimits* vk_limits = &selected_adapter->device_properties.properties.limits;

	limits->max_texture_dimension_1d = vk_limits->maxImageDimension1D;
	limits->max_texture_dimension_2d = vk_limits->maxImageDimension2D;
	limits->max_texture_dimension_3d = vk_limits->maxImageDimension3D;
	limits->max_texture_array_layers = vk_limits->maxImageArrayLayers;
	limits->max_bind_groups = vk_limits->maxBoundDescriptorSets;
	limits->max_bindings_per_bind_group = vk_limits->maxDescriptorSetUniformBuffers;
	limits->max_uniform_buffer_size = vk_limits->maxUniformBufferRange;
	limits->max_storage_buffer_size = vk_limits->maxStorageBufferRange;
	limits->max_vertex_buffers = vk_limits->maxVertexInputBindings;
	limits->max_vertex_attributes = vk_limits->maxVertexInputAttributes;
	limits->max_vertex_buffer_stride = vk_limits->maxVertexInputBindingStride;
	limits->max_color_attachments = vk_limits->maxColorAttachments;
	limits->max_compute_workgroup_size_x = vk_limits->maxComputeWorkGroupSize[0];
	limits->max_compute_workgroup_size_y = vk_limits->maxComputeWorkGroupSize[1];
	limits->max_compute_workgroup_size_z = vk_limits->maxComputeWorkGroupSize[2];
	limits->max_compute_workgroups_per_dimension = vk_limits->maxComputeWorkGroupCount[0];
	limits->max_push_constant_size = vk_limits->maxPushConstantsSize;
}

// Buffer
static ZiBufferHandle zi_vulkan_buffer_create(const ZiBufferDesc* desc) {
	ZiVulkanBuffer* vk_buffer = zi_mem_alloc(sizeof(ZiVulkanBuffer));
	memset(vk_buffer, 0, sizeof(ZiVulkanBuffer));

	VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	buffer_info.size = desc->size;
	buffer_info.usage = zi_buffer_usage_to_vk(desc->usage);
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (has_buffer_device_address) {
		buffer_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}

	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = zi_memory_usage_to_vma(desc->memory);

	VkResult res = vmaCreateBuffer(vma_allocator, &buffer_info, &alloc_info, &vk_buffer->buffer, &vk_buffer->allocation, ZI_NULL);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create buffer: %s", string_VkResult(res));
		zi_mem_free(vk_buffer);
		return (ZiBufferHandle){0};
	}

	vk_buffer->size = desc->size;
	vk_buffer->usage = desc->usage;
	vk_buffer->memory = desc->memory;

	return (ZiBufferHandle){.handler = vk_buffer};
}

static void zi_vulkan_buffer_destroy(ZiBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)handle.handler;
	vmaDestroyBuffer(vma_allocator, vk_buffer->buffer, vk_buffer->allocation);
	zi_mem_free(vk_buffer);
}

static void zi_vulkan_buffer_write(ZiBufferHandle handle, u64 offset, const void* data, u64 size) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)handle.handler;

	void* mapped;
	VkResult res = vmaMapMemory(vma_allocator, vk_buffer->allocation, &mapped);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to map buffer memory: %s", string_VkResult(res));
		return;
	}

	memcpy((u8*)mapped + offset, data, size);
	vmaUnmapMemory(vma_allocator, vk_buffer->allocation);
}

static void* zi_vulkan_buffer_map(ZiBufferHandle handle, u64 offset, u64 size) {
	if (handle.handler == ZI_NULL) return ZI_NULL;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)handle.handler;

	void* mapped;
	VkResult res = vmaMapMemory(vma_allocator, vk_buffer->allocation, &mapped);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to map buffer memory: %s", string_VkResult(res));
		return ZI_NULL;
	}

	return (u8*)mapped + offset;
}

static void zi_vulkan_buffer_unmap(ZiBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)handle.handler;
	vmaUnmapMemory(vma_allocator, vk_buffer->allocation);
}

// Texture
static ZiTextureHandle zi_vulkan_texture_create(const ZiTextureDesc* desc) {
	ZiVulkanTexture* vk_texture = zi_mem_alloc(sizeof(ZiVulkanTexture));
	memset(vk_texture, 0, sizeof(ZiVulkanTexture));

	VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_info.imageType = zi_dimension_to_vk_image_type(desc->dimension);
	image_info.format = zi_format_to_vk(desc->format);
	image_info.extent.width = desc->width;
	image_info.extent.height = desc->height > 0 ? desc->height : 1;
	image_info.extent.depth = desc->depth > 0 ? desc->depth : 1;
	image_info.mipLevels = desc->mip_levels > 0 ? desc->mip_levels : 1;
	image_info.arrayLayers = desc->array_layers > 0 ? desc->array_layers : 1;
	image_info.samples = zi_sample_count_to_vk(desc->sample_count > 0 ? desc->sample_count : 1);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = zi_texture_usage_to_vk(desc->usage);
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (image_info.arrayLayers >= 6 && desc->dimension == ZiTextureDimension_2D) {
		image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkResult res = vmaCreateImage(vma_allocator, &image_info, &alloc_info, &vk_texture->image, &vk_texture->allocation, ZI_NULL);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create image: %s", string_VkResult(res));
		zi_mem_free(vk_texture);
		return (ZiTextureHandle){0};
	}

	vk_texture->format = image_info.format;
	vk_texture->width = desc->width;
	vk_texture->height = image_info.extent.height;
	vk_texture->depth = image_info.extent.depth;
	vk_texture->mip_levels = image_info.mipLevels;
	vk_texture->array_layers = image_info.arrayLayers;
	vk_texture->sample_count = desc->sample_count > 0 ? desc->sample_count : 1;
	vk_texture->dimension = desc->dimension;
	vk_texture->usage = desc->usage;
	vk_texture->is_swapchain_image = ZI_FALSE;

	return (ZiTextureHandle){.handler = vk_texture};
}

static void zi_vulkan_texture_destroy(ZiTextureHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanTexture* vk_texture = (ZiVulkanTexture*)handle.handler;
	if (!vk_texture->is_swapchain_image) {
		vmaDestroyImage(vma_allocator, vk_texture->image, vk_texture->allocation);
	}
	zi_mem_free(vk_texture);
}

// Texture View
static ZiTextureViewHandle zi_vulkan_texture_view_create(const ZiTextureViewDesc* desc) {
	if (desc->texture.handler == ZI_NULL) return (ZiTextureViewHandle){0};

	ZiVulkanTexture* vk_texture = (ZiVulkanTexture*)desc->texture.handler;
	ZiVulkanTextureView* vk_view = zi_mem_alloc(sizeof(ZiVulkanTextureView));
	memset(vk_view, 0, sizeof(ZiVulkanTextureView));

	VkFormat format = desc->format != ZiFormat_Undefined ? zi_format_to_vk(desc->format) : vk_texture->format;

	VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view_info.image = vk_texture->image;
	view_info.viewType = zi_view_dimension_to_vk(desc->dimension);
	view_info.format = format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask = zi_aspect_to_vk(desc->aspect, format);
	view_info.subresourceRange.baseMipLevel = desc->base_mip_level;
	view_info.subresourceRange.levelCount = desc->mip_level_count > 0 ? desc->mip_level_count : VK_REMAINING_MIP_LEVELS;
	view_info.subresourceRange.baseArrayLayer = desc->base_array_layer;
	view_info.subresourceRange.layerCount = desc->array_layer_count > 0 ? desc->array_layer_count : VK_REMAINING_ARRAY_LAYERS;

	VkResult res = vkCreateImageView(device, &view_info, ZI_NULL, &vk_view->view);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create image view: %s", string_VkResult(res));
		zi_mem_free(vk_view);
		return (ZiTextureViewHandle){0};
	}

	vk_view->texture = vk_texture;

	return (ZiTextureViewHandle){.handler = vk_view};
}

static void zi_vulkan_texture_view_destroy(ZiTextureViewHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanTextureView* vk_view = (ZiVulkanTextureView*)handle.handler;
	vkDestroyImageView(device, vk_view->view, ZI_NULL);
	zi_mem_free(vk_view);
}

// Sampler
static ZiSamplerHandle zi_vulkan_sampler_create(const ZiSamplerDesc* desc) {
	ZiVulkanSampler* vk_sampler = zi_mem_alloc(sizeof(ZiVulkanSampler));
	memset(vk_sampler, 0, sizeof(ZiVulkanSampler));

	VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler_info.magFilter = zi_filter_to_vk(desc->mag_filter);
	sampler_info.minFilter = zi_filter_to_vk(desc->min_filter);
	sampler_info.mipmapMode = zi_mipmap_filter_to_vk(desc->mipmap_filter);
	sampler_info.addressModeU = zi_address_mode_to_vk(desc->address_u);
	sampler_info.addressModeV = zi_address_mode_to_vk(desc->address_v);
	sampler_info.addressModeW = zi_address_mode_to_vk(desc->address_w);
	sampler_info.mipLodBias = 0.0f;
	sampler_info.anisotropyEnable = desc->max_anisotropy > 1.0f ? VK_TRUE : VK_FALSE;
	sampler_info.maxAnisotropy = desc->max_anisotropy;
	sampler_info.compareEnable = desc->compare != ZiCompareFunc_Always ? VK_TRUE : VK_FALSE;
	sampler_info.compareOp = zi_compare_to_vk(desc->compare);
	sampler_info.minLod = desc->min_lod;
	sampler_info.maxLod = desc->max_lod > 0 ? desc->max_lod : VK_LOD_CLAMP_NONE;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;

	VkResult res = vkCreateSampler(device, &sampler_info, ZI_NULL, &vk_sampler->sampler);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create sampler: %s", string_VkResult(res));
		zi_mem_free(vk_sampler);
		return (ZiSamplerHandle){0};
	}

	return (ZiSamplerHandle){.handler = vk_sampler};
}

static void zi_vulkan_sampler_destroy(ZiSamplerHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanSampler* vk_sampler = (ZiVulkanSampler*)handle.handler;
	vkDestroySampler(device, vk_sampler->sampler, ZI_NULL);
	zi_mem_free(vk_sampler);
}

// Shader
static ZiShaderHandle zi_vulkan_shader_create(const ZiShaderDesc* desc) {
	ZiVulkanShader* vk_shader = zi_mem_alloc(sizeof(ZiVulkanShader));
	memset(vk_shader, 0, sizeof(ZiVulkanShader));

	VkShaderModuleCreateInfo module_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	module_info.codeSize = desc->code_size;
	module_info.pCode = (const u32*)desc->code;

	VkResult res = vkCreateShaderModule(device, &module_info, ZI_NULL, &vk_shader->module);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create shader module: %s", string_VkResult(res));
		zi_mem_free(vk_shader);
		return (ZiShaderHandle){0};
	}

	vk_shader->stage = desc->stage;
	vk_shader->entry_point = desc->entry_point ? desc->entry_point : "main";

	return (ZiShaderHandle){.handler = vk_shader};
}

static void zi_vulkan_shader_destroy(ZiShaderHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanShader* vk_shader = (ZiVulkanShader*)handle.handler;
	vkDestroyShaderModule(device, vk_shader->module, ZI_NULL);
	zi_mem_free(vk_shader);
}

// Pipeline Layout
static ZiPipelineLayoutHandle zi_vulkan_pipeline_layout_create(const ZiPipelineLayoutDesc* desc) {
	ZiVulkanPipelineLayout* vk_layout = zi_mem_alloc(sizeof(ZiVulkanPipelineLayout));
	memset(vk_layout, 0, sizeof(ZiVulkanPipelineLayout));

	VkDescriptorSetLayout* set_layouts = ZI_NULL;
	if (desc->bind_group_layout_count > 0) {
		set_layouts = zi_mem_alloc(sizeof(VkDescriptorSetLayout) * desc->bind_group_layout_count);
		for (u32 i = 0; i < desc->bind_group_layout_count; ++i) {
			ZiVulkanBindGroupLayout* bg_layout = (ZiVulkanBindGroupLayout*)desc->bind_group_layouts[i].handler;
			set_layouts[i] = bg_layout->layout;
		}
	}

	VkPushConstantRange push_constant = {0};
	push_constant.stageFlags = VK_SHADER_STAGE_ALL;
	push_constant.offset = 0;
	push_constant.size = 128;

	VkPipelineLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	layout_info.setLayoutCount = desc->bind_group_layout_count;
	layout_info.pSetLayouts = set_layouts;
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = &push_constant;

	VkResult res = vkCreatePipelineLayout(device, &layout_info, ZI_NULL, &vk_layout->layout);
	if (set_layouts) zi_mem_free(set_layouts);

	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create pipeline layout: %s", string_VkResult(res));
		zi_mem_free(vk_layout);
		return (ZiPipelineLayoutHandle){0};
	}

	vk_layout->push_constant_size = push_constant.size;

	return (ZiPipelineLayoutHandle){.handler = vk_layout};
}

static void zi_vulkan_pipeline_layout_destroy(ZiPipelineLayoutHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanPipelineLayout* vk_layout = (ZiVulkanPipelineLayout*)handle.handler;
	vkDestroyPipelineLayout(device, vk_layout->layout, ZI_NULL);
	zi_mem_free(vk_layout);
}

// Graphics Pipeline
static ZiPipelineHandle zi_vulkan_graphics_pipeline_create(const ZiGraphicsPipelineDesc* desc) {
	ZiVulkanPipeline* vk_pipeline = zi_mem_alloc(sizeof(ZiVulkanPipeline));
	memset(vk_pipeline, 0, sizeof(ZiVulkanPipeline));

	ZiVulkanPipelineLayout* vk_layout = (ZiVulkanPipelineLayout*)desc->layout.handler;
	ZiVulkanShader* vk_vertex = (ZiVulkanShader*)desc->vertex_shader.handler;
	ZiVulkanShader* vk_fragment = desc->fragment_shader.handler ? (ZiVulkanShader*)desc->fragment_shader.handler : ZI_NULL;

	VkPipelineShaderStageCreateInfo shader_stages[2];
	u32 shader_stage_count = 0;

	shader_stages[shader_stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[shader_stage_count].pNext = ZI_NULL;
	shader_stages[shader_stage_count].flags = 0;
	shader_stages[shader_stage_count].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[shader_stage_count].module = vk_vertex->module;
	shader_stages[shader_stage_count].pName = vk_vertex->entry_point;
	shader_stages[shader_stage_count].pSpecializationInfo = ZI_NULL;
	shader_stage_count++;

	if (vk_fragment) {
		shader_stages[shader_stage_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[shader_stage_count].pNext = ZI_NULL;
		shader_stages[shader_stage_count].flags = 0;
		shader_stages[shader_stage_count].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[shader_stage_count].module = vk_fragment->module;
		shader_stages[shader_stage_count].pName = vk_fragment->entry_point;
		shader_stages[shader_stage_count].pSpecializationInfo = ZI_NULL;
		shader_stage_count++;
	}

	VkVertexInputBindingDescription* bindings = ZI_NULL;
	if (desc->vertex_binding_count > 0) {
		bindings = zi_mem_alloc(sizeof(VkVertexInputBindingDescription) * desc->vertex_binding_count);
		for (u32 i = 0; i < desc->vertex_binding_count; ++i) {
			bindings[i].binding = desc->vertex_bindings[i].binding;
			bindings[i].stride = desc->vertex_bindings[i].stride;
			bindings[i].inputRate = desc->vertex_bindings[i].input_rate == ZiVertexInputRate_Instance ?
				VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		}
	}

	VkVertexInputAttributeDescription* attributes = ZI_NULL;
	if (desc->vertex_attribute_count > 0) {
		attributes = zi_mem_alloc(sizeof(VkVertexInputAttributeDescription) * desc->vertex_attribute_count);
		for (u32 i = 0; i < desc->vertex_attribute_count; ++i) {
			attributes[i].location = desc->vertex_attributes[i].location;
			attributes[i].binding = desc->vertex_attributes[i].binding;
			attributes[i].format = zi_format_to_vk(desc->vertex_attributes[i].format);
			attributes[i].offset = desc->vertex_attributes[i].offset;
		}
	}

	VkPipelineVertexInputStateCreateInfo vertex_input = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertex_input.vertexBindingDescriptionCount = desc->vertex_binding_count;
	vertex_input.pVertexBindingDescriptions = bindings;
	vertex_input.vertexAttributeDescriptionCount = desc->vertex_attribute_count;
	vertex_input.pVertexAttributeDescriptions = attributes;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	input_assembly.topology = zi_topology_to_vk(desc->topology);
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = desc->rasterizer.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = zi_cull_mode_to_vk(desc->rasterizer.cull_mode);
	rasterizer.frontFace = zi_front_face_to_vk(desc->rasterizer.front_face);
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depth_stencil.depthTestEnable = desc->depth_stencil.depth_test_enable ? VK_TRUE : VK_FALSE;
	depth_stencil.depthWriteEnable = desc->depth_stencil.depth_write_enable ? VK_TRUE : VK_FALSE;
	depth_stencil.depthCompareOp = zi_compare_to_vk(desc->depth_stencil.depth_compare);
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = desc->blend.blend_enable ? VK_TRUE : VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = zi_blend_factor_to_vk(desc->blend.src_color);
	color_blend_attachment.dstColorBlendFactor = zi_blend_factor_to_vk(desc->blend.dst_color);
	color_blend_attachment.colorBlendOp = zi_blend_op_to_vk(desc->blend.color_op);
	color_blend_attachment.srcAlphaBlendFactor = zi_blend_factor_to_vk(desc->blend.src_alpha);
	color_blend_attachment.dstAlphaBlendFactor = zi_blend_factor_to_vk(desc->blend.dst_alpha);
	color_blend_attachment.alphaBlendOp = zi_blend_op_to_vk(desc->blend.alpha_op);

	VkPipelineColorBlendStateCreateInfo color_blending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	VkFormat color_format = zi_format_to_vk(desc->color_format);
	VkFormat depth_format = zi_format_to_vk(desc->depth_format);

	VkPipelineRenderingCreateInfo rendering_info = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
	rendering_info.colorAttachmentCount = desc->color_format != ZiFormat_Undefined ? 1 : 0;
	rendering_info.pColorAttachmentFormats = desc->color_format != ZiFormat_Undefined ? &color_format : ZI_NULL;
	rendering_info.depthAttachmentFormat = depth_format;
	rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	VkGraphicsPipelineCreateInfo pipeline_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipeline_info.pNext = &rendering_info;
	pipeline_info.stageCount = shader_stage_count;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = vk_layout->layout;
	pipeline_info.renderPass = VK_NULL_HANDLE;
	pipeline_info.subpass = 0;

	VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, ZI_NULL, &vk_pipeline->pipeline);

	if (bindings) zi_mem_free(bindings);
	if (attributes) zi_mem_free(attributes);

	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create graphics pipeline: %s", string_VkResult(res));
		zi_mem_free(vk_pipeline);
		return (ZiPipelineHandle){0};
	}

	vk_pipeline->layout = vk_layout->layout;
	vk_pipeline->bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

	return (ZiPipelineHandle){.handler = vk_pipeline};
}

static void zi_vulkan_graphics_pipeline_destroy(ZiPipelineHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanPipeline* vk_pipeline = (ZiVulkanPipeline*)handle.handler;
	vkDestroyPipeline(device, vk_pipeline->pipeline, ZI_NULL);
	zi_mem_free(vk_pipeline);
}

// Compute Pipeline
static ZiPipelineHandle zi_vulkan_compute_pipeline_create(const ZiComputePipelineDesc* desc) {
	ZiVulkanPipeline* vk_pipeline = zi_mem_alloc(sizeof(ZiVulkanPipeline));
	memset(vk_pipeline, 0, sizeof(ZiVulkanPipeline));

	ZiVulkanPipelineLayout* vk_layout = (ZiVulkanPipelineLayout*)desc->layout.handler;
	ZiVulkanShader* vk_compute = (ZiVulkanShader*)desc->compute_shader.handler;

	VkPipelineShaderStageCreateInfo shader_stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage.module = vk_compute->module;
	shader_stage.pName = vk_compute->entry_point;

	VkComputePipelineCreateInfo pipeline_info = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	pipeline_info.stage = shader_stage;
	pipeline_info.layout = vk_layout->layout;

	VkResult res = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, ZI_NULL, &vk_pipeline->pipeline);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create compute pipeline: %s", string_VkResult(res));
		zi_mem_free(vk_pipeline);
		return (ZiPipelineHandle){0};
	}

	vk_pipeline->layout = vk_layout->layout;
	vk_pipeline->bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;

	return (ZiPipelineHandle){.handler = vk_pipeline};
}

static void zi_vulkan_compute_pipeline_destroy(ZiPipelineHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanPipeline* vk_pipeline = (ZiVulkanPipeline*)handle.handler;
	vkDestroyPipeline(device, vk_pipeline->pipeline, ZI_NULL);
	zi_mem_free(vk_pipeline);
}

// Bind Group Layout
static ZiBindGroupLayoutHandle zi_vulkan_bind_group_layout_create(const ZiBindGroupLayoutDesc* desc) {
	ZiVulkanBindGroupLayout* vk_layout = zi_mem_alloc(sizeof(ZiVulkanBindGroupLayout));
	memset(vk_layout, 0, sizeof(ZiVulkanBindGroupLayout));

	VkDescriptorSetLayoutBinding* bindings = ZI_NULL;
	if (desc->entry_count > 0) {
		bindings = zi_mem_alloc(sizeof(VkDescriptorSetLayoutBinding) * desc->entry_count);
		for (u32 i = 0; i < desc->entry_count; ++i) {
			bindings[i].binding = desc->entries[i].binding;
			bindings[i].descriptorType = zi_binding_type_to_vk(desc->entries[i].type);
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = zi_shader_stage_to_vk(desc->entries[i].visibility);
			bindings[i].pImmutableSamplers = ZI_NULL;
		}
	}

	VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	layout_info.bindingCount = desc->entry_count;
	layout_info.pBindings = bindings;

	VkResult res = vkCreateDescriptorSetLayout(device, &layout_info, ZI_NULL, &vk_layout->layout);
	if (bindings) zi_mem_free(bindings);

	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create descriptor set layout: %s", string_VkResult(res));
		zi_mem_free(vk_layout);
		return (ZiBindGroupLayoutHandle){0};
	}

	vk_layout->binding_count = desc->entry_count;

	return (ZiBindGroupLayoutHandle){.handler = vk_layout};
}

static void zi_vulkan_bind_group_layout_destroy(ZiBindGroupLayoutHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanBindGroupLayout* vk_layout = (ZiVulkanBindGroupLayout*)handle.handler;
	vkDestroyDescriptorSetLayout(device, vk_layout->layout, ZI_NULL);
	zi_mem_free(vk_layout);
}

// Bind Group
static ZiBindGroupHandle zi_vulkan_bind_group_create(const ZiBindGroupDesc* desc) {
	if (desc->layout.handler == ZI_NULL) return (ZiBindGroupHandle){0};

	ZiVulkanBindGroup* vk_bind_group = zi_mem_alloc(sizeof(ZiVulkanBindGroup));
	memset(vk_bind_group, 0, sizeof(ZiVulkanBindGroup));

	ZiVulkanBindGroupLayout* vk_layout = (ZiVulkanBindGroupLayout*)desc->layout.handler;

	VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &vk_layout->layout;

	VkResult res = vkAllocateDescriptorSets(device, &alloc_info, &vk_bind_group->set);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to allocate descriptor set: %s", string_VkResult(res));
		zi_mem_free(vk_bind_group);
		return (ZiBindGroupHandle){0};
	}

	if (desc->entry_count > 0) {
		VkWriteDescriptorSet* writes = zi_mem_alloc(sizeof(VkWriteDescriptorSet) * desc->entry_count);
		VkDescriptorBufferInfo* buffer_infos = zi_mem_alloc(sizeof(VkDescriptorBufferInfo) * desc->entry_count);
		VkDescriptorImageInfo* image_infos = zi_mem_alloc(sizeof(VkDescriptorImageInfo) * desc->entry_count);

		for (u32 i = 0; i < desc->entry_count; ++i) {
			memset(&writes[i], 0, sizeof(VkWriteDescriptorSet));
			memset(&buffer_infos[i], 0, sizeof(VkDescriptorBufferInfo));
			memset(&image_infos[i], 0, sizeof(VkDescriptorImageInfo));

			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = vk_bind_group->set;
			writes[i].dstBinding = desc->entries[i].binding;
			writes[i].dstArrayElement = 0;
			writes[i].descriptorCount = 1;

			if (desc->entries[i].buffer.handler != ZI_NULL) {
				ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)desc->entries[i].buffer.handler;
				buffer_infos[i].buffer = vk_buffer->buffer;
				buffer_infos[i].offset = desc->entries[i].offset;
				buffer_infos[i].range = desc->entries[i].size > 0 ? desc->entries[i].size : VK_WHOLE_SIZE;
				writes[i].pBufferInfo = &buffer_infos[i];

				if (vk_buffer->usage & ZiBufferUsage_Uniform) {
					writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				} else {
					writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				}
			} else if (desc->entries[i].texture.handler != ZI_NULL) {
				ZiVulkanTexture* vk_texture = (ZiVulkanTexture*)desc->entries[i].texture.handler;
				image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				writes[i].pImageInfo = &image_infos[i];

				if (vk_texture->usage & ZiTextureUsage_Storage) {
					writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					image_infos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				} else {
					writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				}
			} else if (desc->entries[i].sampler.handler != ZI_NULL) {
				ZiVulkanSampler* vk_sampler = (ZiVulkanSampler*)desc->entries[i].sampler.handler;
				image_infos[i].sampler = vk_sampler->sampler;
				writes[i].pImageInfo = &image_infos[i];
				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			}
		}

		vkUpdateDescriptorSets(device, desc->entry_count, writes, 0, ZI_NULL);

		zi_mem_free(writes);
		zi_mem_free(buffer_infos);
		zi_mem_free(image_infos);
	}

	return (ZiBindGroupHandle){.handler = vk_bind_group};
}

static void zi_vulkan_bind_group_destroy(ZiBindGroupHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanBindGroup* vk_bind_group = (ZiVulkanBindGroup*)handle.handler;
	vkFreeDescriptorSets(device, descriptor_pool, 1, &vk_bind_group->set);
	zi_mem_free(vk_bind_group);
}

// Render Pass
static ZiRenderPassHandle zi_vulkan_render_pass_create(const ZiRenderPassDesc* desc) {
	ZiVulkanRenderPass* vk_rp = zi_mem_alloc(sizeof(ZiVulkanRenderPass));
	memset(vk_rp, 0, sizeof(ZiVulkanRenderPass));

	u32 total_attachments = desc->color_attachment_count + (desc->depth_attachment ? 1 : 0);
	VkAttachmentDescription* attachments = zi_mem_alloc(sizeof(VkAttachmentDescription) * total_attachments);
	memset(attachments, 0, sizeof(VkAttachmentDescription) * total_attachments);

	VkAttachmentReference* color_refs = ZI_NULL;
	if (desc->color_attachment_count > 0) {
		color_refs = zi_mem_alloc(sizeof(VkAttachmentReference) * desc->color_attachment_count);
	}

	for (u32 i = 0; i < desc->color_attachment_count; ++i) {
		attachments[i].format = zi_format_to_vk(desc->color_attachments[i].format);
		attachments[i].samples = desc->color_attachments[i].sample_count > 1 ? desc->color_attachments[i].sample_count : VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = zi_load_op_to_vk(desc->color_attachments[i].load_op);
		attachments[i].storeOp = zi_store_op_to_vk(desc->color_attachments[i].store_op);
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		color_refs[i].attachment = i;
		color_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depth_ref = {0};
	if (desc->depth_attachment) {
		u32 depth_idx = desc->color_attachment_count;
		attachments[depth_idx].format = zi_format_to_vk(desc->depth_attachment->format);
		attachments[depth_idx].samples = desc->depth_attachment->sample_count > 1 ? desc->depth_attachment->sample_count : VK_SAMPLE_COUNT_1_BIT;
		attachments[depth_idx].loadOp = zi_load_op_to_vk(desc->depth_attachment->depth_load_op);
		attachments[depth_idx].storeOp = zi_store_op_to_vk(desc->depth_attachment->depth_store_op);
		attachments[depth_idx].stencilLoadOp = zi_load_op_to_vk(desc->depth_attachment->stencil_load_op);
		attachments[depth_idx].stencilStoreOp = zi_store_op_to_vk(desc->depth_attachment->stencil_store_op);
		attachments[depth_idx].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[depth_idx].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depth_ref.attachment = depth_idx;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = desc->color_attachment_count;
	subpass.pColorAttachments = color_refs;
	subpass.pDepthStencilAttachment = desc->depth_attachment ? &depth_ref : ZI_NULL;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	rp_info.attachmentCount = total_attachments;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 1;
	rp_info.pDependencies = &dependency;

	VkResult res = vkCreateRenderPass(device, &rp_info, ZI_NULL, &vk_rp->render_pass);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create render pass: %s", string_VkResult(res));
		zi_mem_free(attachments);
		if (color_refs) zi_mem_free(color_refs);
		zi_mem_free(vk_rp);
		return (ZiRenderPassHandle){0};
	}

	vk_rp->color_attachment_count = desc->color_attachment_count;
	vk_rp->has_depth_attachment = desc->depth_attachment != ZI_NULL;

	zi_mem_free(attachments);
	if (color_refs) zi_mem_free(color_refs);

	return (ZiRenderPassHandle){.handler = vk_rp};
}

static void zi_vulkan_render_pass_destroy(ZiRenderPassHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanRenderPass* vk_rp = (ZiVulkanRenderPass*)handle.handler;
	vkDestroyRenderPass(device, vk_rp->render_pass, ZI_NULL);
	zi_mem_free(vk_rp);
}

// Framebuffer
static ZiFramebufferHandle zi_vulkan_framebuffer_create(const ZiFramebufferDesc* desc) {
	if (desc->render_pass.handler == ZI_NULL) {
		zi_log_error("Framebuffer requires a valid render pass");
		return (ZiFramebufferHandle){0};
	}

	ZiVulkanFramebuffer* vk_fb = zi_mem_alloc(sizeof(ZiVulkanFramebuffer));
	memset(vk_fb, 0, sizeof(ZiVulkanFramebuffer));

	ZiVulkanRenderPass* vk_rp = (ZiVulkanRenderPass*)desc->render_pass.handler;

	u32 total_attachments = desc->color_attachment_count + (desc->depth_attachment ? 1 : 0);
	VkImageView* views = zi_mem_alloc(sizeof(VkImageView) * total_attachments);

	for (u32 i = 0; i < desc->color_attachment_count; ++i) {
		ZiVulkanTextureView* vk_view = (ZiVulkanTextureView*)desc->color_attachments[i].handler;
		views[i] = vk_view->view;
	}

	if (desc->depth_attachment) {
		ZiVulkanTextureView* vk_depth_view = (ZiVulkanTextureView*)desc->depth_attachment->handler;
		views[desc->color_attachment_count] = vk_depth_view->view;
	}

	VkFramebufferCreateInfo fb_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass = vk_rp->render_pass;
	fb_info.attachmentCount = total_attachments;
	fb_info.pAttachments = views;
	fb_info.width = desc->width;
	fb_info.height = desc->height;
	fb_info.layers = desc->layers > 0 ? desc->layers : 1;

	VkResult res = vkCreateFramebuffer(device, &fb_info, ZI_NULL, &vk_fb->framebuffer);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create framebuffer: %s", string_VkResult(res));
		zi_mem_free(views);
		zi_mem_free(vk_fb);
		return (ZiFramebufferHandle){0};
	}

	vk_fb->width = desc->width;
	vk_fb->height = desc->height;
	vk_fb->layers = desc->layers > 0 ? desc->layers : 1;

	zi_mem_free(views);

	return (ZiFramebufferHandle){.handler = vk_fb};
}

static void zi_vulkan_framebuffer_destroy(ZiFramebufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanFramebuffer* vk_fb = (ZiVulkanFramebuffer*)handle.handler;
	vkDestroyFramebuffer(device, vk_fb->framebuffer, ZI_NULL);
	zi_mem_free(vk_fb);
}

// Command Buffer
static ZiCommandBufferHandle zi_vulkan_command_buffer_create() {
	ZiVulkanCommandBuffer* vk_cmd = zi_mem_alloc(sizeof(ZiVulkanCommandBuffer));
	memset(vk_cmd, 0, sizeof(ZiVulkanCommandBuffer));

	VkCommandPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pool_info.queueFamilyIndex = selected_adapter->graphics_family;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult res = vkCreateCommandPool(device, &pool_info, ZI_NULL, &vk_cmd->pool);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create command pool: %s", string_VkResult(res));
		zi_mem_free(vk_cmd);
		return (ZiCommandBufferHandle){0};
	}

	VkCommandBufferAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	alloc_info.commandPool = vk_cmd->pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	res = vkAllocateCommandBuffers(device, &alloc_info, &vk_cmd->cmd);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to allocate command buffer: %s", string_VkResult(res));
		vkDestroyCommandPool(device, vk_cmd->pool, ZI_NULL);
		zi_mem_free(vk_cmd);
		return (ZiCommandBufferHandle){0};
	}

	VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	res = vkCreateFence(device, &fence_info, ZI_NULL, &vk_cmd->fence);
	if (res != VK_SUCCESS) {
		zi_log_error("Failed to create fence: %s", string_VkResult(res));
		vkDestroyCommandPool(device, vk_cmd->pool, ZI_NULL);
		zi_mem_free(vk_cmd);
		return (ZiCommandBufferHandle){0};
	}

	vk_cmd->is_recording = ZI_FALSE;

	return (ZiCommandBufferHandle){.handler = vk_cmd};
}

static void zi_vulkan_command_buffer_destroy(ZiCommandBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)handle.handler;
	vkDestroyFence(device, vk_cmd->fence, ZI_NULL);
	vkDestroyCommandPool(device, vk_cmd->pool, ZI_NULL);
	zi_mem_free(vk_cmd);
}

static void zi_vulkan_command_buffer_begin(ZiCommandBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)handle.handler;

	vkResetCommandBuffer(vk_cmd->cmd, 0);

	VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(vk_cmd->cmd, &begin_info);
	vk_cmd->is_recording = ZI_TRUE;
}

static void zi_vulkan_command_buffer_end(ZiCommandBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)handle.handler;
	vkEndCommandBuffer(vk_cmd->cmd);
	vk_cmd->is_recording = ZI_FALSE;
}

static void zi_vulkan_command_buffer_submit(ZiCommandBufferHandle handle) {
	if (handle.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)handle.handler;

	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vk_cmd->cmd;

	vkQueueSubmit(graphics_queue, 1, &submit_info, vk_cmd->fence);
}

// Command Buffer - Render Pass
static VkPipelineLayout current_pipeline_layout = VK_NULL_HANDLE;

static void zi_vulkan_cmd_begin_render_pass(ZiCommandBufferHandle cmd, const ZiRenderPassBeginDesc* desc) {
	if (cmd.handler == ZI_NULL) return;
	if (desc->render_pass.handler == ZI_NULL || desc->framebuffer.handler == ZI_NULL) return;

	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanRenderPass* vk_rp = (ZiVulkanRenderPass*)desc->render_pass.handler;
	ZiVulkanFramebuffer* vk_fb = (ZiVulkanFramebuffer*)desc->framebuffer.handler;

	u32 total_clear_values = vk_rp->color_attachment_count + (vk_rp->has_depth_attachment ? 1 : 0);
	VkClearValue* clear_values = zi_mem_alloc(sizeof(VkClearValue) * total_clear_values);
	memset(clear_values, 0, sizeof(VkClearValue) * total_clear_values);

	for (u32 i = 0; i < vk_rp->color_attachment_count && i < desc->clear_color_count; ++i) {
		clear_values[i].color.float32[0] = desc->clear_colors[i * 4 + 0];
		clear_values[i].color.float32[1] = desc->clear_colors[i * 4 + 1];
		clear_values[i].color.float32[2] = desc->clear_colors[i * 4 + 2];
		clear_values[i].color.float32[3] = desc->clear_colors[i * 4 + 3];
	}

	if (vk_rp->has_depth_attachment) {
		clear_values[vk_rp->color_attachment_count].depthStencil.depth = desc->clear_depth;
		clear_values[vk_rp->color_attachment_count].depthStencil.stencil = desc->clear_stencil;
	}

	VkRenderPassBeginInfo rp_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	rp_begin_info.renderPass = vk_rp->render_pass;
	rp_begin_info.framebuffer = vk_fb->framebuffer;
	rp_begin_info.renderArea.offset = (VkOffset2D){0, 0};
	rp_begin_info.renderArea.extent = (VkExtent2D){vk_fb->width, vk_fb->height};
	rp_begin_info.clearValueCount = total_clear_values;
	rp_begin_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(vk_cmd->cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	zi_mem_free(clear_values);
}

static void zi_vulkan_cmd_end_render_pass(ZiCommandBufferHandle cmd) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdEndRenderPass(vk_cmd->cmd);
}

// Command Buffer - State
static void zi_vulkan_cmd_set_pipeline(ZiCommandBufferHandle cmd, ZiPipelineHandle pipeline) {
	if (cmd.handler == ZI_NULL || pipeline.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanPipeline* vk_pipeline = (ZiVulkanPipeline*)pipeline.handler;
	vkCmdBindPipeline(vk_cmd->cmd, vk_pipeline->bind_point, vk_pipeline->pipeline);
	current_pipeline_layout = vk_pipeline->layout;
}

static void zi_vulkan_cmd_set_bind_group(ZiCommandBufferHandle cmd, u32 index, ZiBindGroupHandle bind_group) {
	if (cmd.handler == ZI_NULL || bind_group.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBindGroup* vk_bg = (ZiVulkanBindGroup*)bind_group.handler;
	vkCmdBindDescriptorSets(vk_cmd->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline_layout, index, 1, &vk_bg->set, 0, ZI_NULL);
}

static void zi_vulkan_cmd_set_vertex_buffer(ZiCommandBufferHandle cmd, u32 slot, ZiBufferHandle buffer, u64 offset) {
	if (cmd.handler == ZI_NULL || buffer.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)buffer.handler;
	VkDeviceSize offsets[] = {offset};
	vkCmdBindVertexBuffers(vk_cmd->cmd, slot, 1, &vk_buffer->buffer, offsets);
}

static void zi_vulkan_cmd_set_index_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, ZiIndexFormat format) {
	if (cmd.handler == ZI_NULL || buffer.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)buffer.handler;
	vkCmdBindIndexBuffer(vk_cmd->cmd, vk_buffer->buffer, offset, zi_index_format_to_vk(format));
}

static void zi_vulkan_cmd_push_constants(ZiCommandBufferHandle cmd, ZiShaderStage stages, u32 offset, u32 size, const void* data) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdPushConstants(vk_cmd->cmd, current_pipeline_layout, zi_shader_stage_to_vk(stages), offset, size, data);
}

static void zi_vulkan_cmd_set_viewport(ZiCommandBufferHandle cmd, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	VkViewport viewport = {x, y, width, height, min_depth, max_depth};
	vkCmdSetViewport(vk_cmd->cmd, 0, 1, &viewport);
}

static void zi_vulkan_cmd_set_scissor(ZiCommandBufferHandle cmd, u32 x, u32 y, u32 width, u32 height) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	VkRect2D scissor = {{(i32)x, (i32)y}, {width, height}};
	vkCmdSetScissor(vk_cmd->cmd, 0, 1, &scissor);
}

static void zi_vulkan_cmd_set_blend_constant(ZiCommandBufferHandle cmd, f32 color[4]) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdSetBlendConstants(vk_cmd->cmd, color);
}

static void zi_vulkan_cmd_set_stencil_reference(ZiCommandBufferHandle cmd, u32 reference) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdSetStencilReference(vk_cmd->cmd, VK_STENCIL_FACE_FRONT_AND_BACK, reference);
}

// Command Buffer - Draw
static void zi_vulkan_cmd_draw(ZiCommandBufferHandle cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdDraw(vk_cmd->cmd, vertex_count, instance_count, first_vertex, first_instance);
}

static void zi_vulkan_cmd_draw_indexed(ZiCommandBufferHandle cmd, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdDrawIndexed(vk_cmd->cmd, index_count, instance_count, first_index, vertex_offset, first_instance);
}

static void zi_vulkan_cmd_draw_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) {
	if (cmd.handler == ZI_NULL || buffer.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)buffer.handler;
	vkCmdDrawIndirect(vk_cmd->cmd, vk_buffer->buffer, offset, draw_count, stride);
}

static void zi_vulkan_cmd_draw_indexed_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) {
	if (cmd.handler == ZI_NULL || buffer.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)buffer.handler;
	vkCmdDrawIndexedIndirect(vk_cmd->cmd, vk_buffer->buffer, offset, draw_count, stride);
}

// Command Buffer - Compute
static void zi_vulkan_cmd_dispatch(ZiCommandBufferHandle cmd, u32 group_count_x, u32 group_count_y, u32 group_count_z) {
	if (cmd.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdDispatch(vk_cmd->cmd, group_count_x, group_count_y, group_count_z);
}

static void zi_vulkan_cmd_dispatch_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset) {
	if (cmd.handler == ZI_NULL || buffer.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_buffer = (ZiVulkanBuffer*)buffer.handler;
	vkCmdDispatchIndirect(vk_cmd->cmd, vk_buffer->buffer, offset);
}

// Command Buffer - Copy
static void zi_vulkan_cmd_copy_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiBufferHandle dst, u64 dst_offset, u64 size) {
	if (cmd.handler == ZI_NULL || src.handler == ZI_NULL || dst.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_src = (ZiVulkanBuffer*)src.handler;
	ZiVulkanBuffer* vk_dst = (ZiVulkanBuffer*)dst.handler;

	VkBufferCopy region = {src_offset, dst_offset, size};
	vkCmdCopyBuffer(vk_cmd->cmd, vk_src->buffer, vk_dst->buffer, 1, &region);
}

static void zi_vulkan_cmd_copy_texture(ZiCommandBufferHandle cmd, ZiTextureHandle src, ZiTextureHandle dst) {
	if (cmd.handler == ZI_NULL || src.handler == ZI_NULL || dst.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanTexture* vk_src = (ZiVulkanTexture*)src.handler;
	ZiVulkanTexture* vk_dst = (ZiVulkanTexture*)dst.handler;

	VkImageCopy region = {0};
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = vk_src->array_layers;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.mipLevel = 0;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = vk_dst->array_layers;
	region.extent.width = vk_src->width;
	region.extent.height = vk_src->height;
	region.extent.depth = vk_src->depth;

	vkCmdCopyImage(vk_cmd->cmd, vk_src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	               vk_dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

static void zi_vulkan_cmd_copy_buffer_to_texture(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiTextureHandle dst, u32 mip_level, u32 array_layer) {
	if (cmd.handler == ZI_NULL || src.handler == ZI_NULL || dst.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanBuffer* vk_src = (ZiVulkanBuffer*)src.handler;
	ZiVulkanTexture* vk_dst = (ZiVulkanTexture*)dst.handler;

	VkBufferImageCopy region = {0};
	region.bufferOffset = src_offset;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = mip_level;
	region.imageSubresource.baseArrayLayer = array_layer;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = (VkOffset3D){0, 0, 0};
	region.imageExtent.width = vk_dst->width >> mip_level;
	region.imageExtent.height = vk_dst->height >> mip_level;
	region.imageExtent.depth = 1;

	if (region.imageExtent.width == 0) region.imageExtent.width = 1;
	if (region.imageExtent.height == 0) region.imageExtent.height = 1;

	vkCmdCopyBufferToImage(vk_cmd->cmd, vk_src->buffer, vk_dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

static void zi_vulkan_cmd_copy_texture_to_buffer(ZiCommandBufferHandle cmd, ZiTextureHandle src, u32 mip_level, u32 array_layer, ZiBufferHandle dst, u64 dst_offset) {
	if (cmd.handler == ZI_NULL || src.handler == ZI_NULL || dst.handler == ZI_NULL) return;
	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	ZiVulkanTexture* vk_src = (ZiVulkanTexture*)src.handler;
	ZiVulkanBuffer* vk_dst = (ZiVulkanBuffer*)dst.handler;

	VkBufferImageCopy region = {0};
	region.bufferOffset = dst_offset;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = mip_level;
	region.imageSubresource.baseArrayLayer = array_layer;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = (VkOffset3D){0, 0, 0};
	region.imageExtent.width = vk_src->width >> mip_level;
	region.imageExtent.height = vk_src->height >> mip_level;
	region.imageExtent.depth = 1;

	if (region.imageExtent.width == 0) region.imageExtent.width = 1;
	if (region.imageExtent.height == 0) region.imageExtent.height = 1;

	vkCmdCopyImageToBuffer(vk_cmd->cmd, vk_src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_dst->buffer, 1, &region);
}

// Swapchain helper: create swapchain internal resources
static ZiBool zi_vulkan_swapchain_create_resources(ZiVulkanSwapchain* sc) {
	// Query surface capabilities
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selected_adapter->device, sc->surface, &capabilities);

	// Determine extent
	if (capabilities.currentExtent.width != U32_MAX) {
		sc->width = capabilities.currentExtent.width;
		sc->height = capabilities.currentExtent.height;
	}

	// Clamp to valid range
	if (sc->width < capabilities.minImageExtent.width) sc->width = capabilities.minImageExtent.width;
	if (sc->width > capabilities.maxImageExtent.width) sc->width = capabilities.maxImageExtent.width;
	if (sc->height < capabilities.minImageExtent.height) sc->height = capabilities.minImageExtent.height;
	if (sc->height > capabilities.maxImageExtent.height) sc->height = capabilities.maxImageExtent.height;

	// Choose image count
	u32 image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	create_info.surface = sc->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = sc->format;
	create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	create_info.imageExtent.width = sc->width;
	create_info.imageExtent.height = sc->height;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	u32 queue_family_indices[2] = {selected_adapter->graphics_family, selected_adapter->present_family};
	if (selected_adapter->graphics_family != selected_adapter->present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = sc->present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(device, &create_info, ZI_NULL, &sc->swapchain);
	if (result != VK_SUCCESS) {
		zi_log_error("Failed to create swapchain");
		return ZI_FALSE;
	}

	// Get swapchain images
	vkGetSwapchainImagesKHR(device, sc->swapchain, &sc->image_count, ZI_NULL);
	sc->images = zi_mem_alloc(sizeof(VkImage) * sc->image_count);
	vkGetSwapchainImagesKHR(device, sc->swapchain, &sc->image_count, sc->images);

	// Create texture wrappers for each image
	sc->textures = zi_mem_alloc(sizeof(ZiVulkanTexture) * sc->image_count);
	for (u32 i = 0; i < sc->image_count; i++) {
		sc->textures[i].image = sc->images[i];
		sc->textures[i].allocation = VK_NULL_HANDLE;
		sc->textures[i].format = sc->format;
		sc->textures[i].width = sc->width;
		sc->textures[i].height = sc->height;
		sc->textures[i].depth = 1;
		sc->textures[i].mip_levels = 1;
		sc->textures[i].array_layers = 1;
		sc->textures[i].sample_count = 1;
		sc->textures[i].dimension = ZiTextureDimension_2D;
		sc->textures[i].usage = ZiTextureUsage_RenderTarget;
		sc->textures[i].is_swapchain_image = ZI_TRUE;
	}

	// Create image views
	sc->image_views = zi_mem_alloc(sizeof(VkImageView) * sc->image_count);
	for (u32 i = 0; i < sc->image_count; i++) {
		VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		view_info.image = sc->images[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = sc->format;
		view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &view_info, ZI_NULL, &sc->image_views[i]);
		if (result != VK_SUCCESS) {
			zi_log_error("Failed to create swapchain image view");
			return ZI_FALSE;
		}
	}

	return ZI_TRUE;
}

static void zi_vulkan_swapchain_destroy_resources(ZiVulkanSwapchain* sc) {
	vkDeviceWaitIdle(device);

	if (sc->image_views) {
		for (u32 i = 0; i < sc->image_count; i++) {
			if (sc->image_views[i]) {
				vkDestroyImageView(device, sc->image_views[i], ZI_NULL);
			}
		}
		zi_mem_free(sc->image_views);
		sc->image_views = ZI_NULL;
	}

	if (sc->textures) {
		zi_mem_free(sc->textures);
		sc->textures = ZI_NULL;
	}

	if (sc->images) {
		zi_mem_free(sc->images);
		sc->images = ZI_NULL;
	}

	if (sc->swapchain) {
		vkDestroySwapchainKHR(device, sc->swapchain, ZI_NULL);
		sc->swapchain = VK_NULL_HANDLE;
	}
}

static ZiSwapchainHandle zi_vulkan_swapchain_create(const ZiSwapchainDesc* desc) {
	ZiVulkanSwapchain* sc = zi_mem_alloc(sizeof(ZiVulkanSwapchain));
	memset(sc, 0, sizeof(ZiVulkanSwapchain));

	// Create surface from window handle
	zi_platform_create_surface(instance, desc->window_handle, &sc->surface);
	if (!sc->surface) {
		zi_log_error("Failed to create surface");
		zi_mem_free(sc);
		return (ZiSwapchainHandle){0};
	}

	// Choose surface format
	u32 format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(selected_adapter->device, sc->surface, &format_count, ZI_NULL);
	VkSurfaceFormatKHR* formats = zi_mem_alloc(sizeof(VkSurfaceFormatKHR) * format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(selected_adapter->device, sc->surface, &format_count, formats);

	VkFormat desired_format = zi_format_to_vk(desc->format);
	sc->format = formats[0].format; // Default to first available
	for (u32 i = 0; i < format_count; i++) {
		if (formats[i].format == desired_format) {
			sc->format = desired_format;
			break;
		}
	}
	zi_mem_free(formats);

	// Choose present mode
	u32 present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(selected_adapter->device, sc->surface, &present_mode_count, ZI_NULL);
	VkPresentModeKHR* present_modes = zi_mem_alloc(sizeof(VkPresentModeKHR) * present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(selected_adapter->device, sc->surface, &present_mode_count, present_modes);

	sc->present_mode = VK_PRESENT_MODE_FIFO_KHR; // Always supported, vsync on
	if (!desc->vsync) {
		for (u32 i = 0; i < present_mode_count; i++) {
			if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				sc->present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				sc->present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}
	zi_mem_free(present_modes);

	// Create semaphores for synchronization
	VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	for (u32 i = 0; i < ZI_FRAMES_IN_FLIGHT; i++) {
		vkCreateSemaphore(device, &semaphore_info, ZI_NULL, &sc->image_available_semaphores[i]);
		vkCreateSemaphore(device, &semaphore_info, ZI_NULL, &sc->render_finished_semaphores[i]);
	}

	sc->current_frame = 0;

	// Create swapchain resources
	if (!zi_vulkan_swapchain_create_resources(sc)) {
		// Cleanup on failure
		for (u32 i = 0; i < ZI_FRAMES_IN_FLIGHT; i++) {
			if (sc->image_available_semaphores[i]) vkDestroySemaphore(device, sc->image_available_semaphores[i], ZI_NULL);
			if (sc->render_finished_semaphores[i]) vkDestroySemaphore(device, sc->render_finished_semaphores[i], ZI_NULL);
		}
		vkDestroySurfaceKHR(instance, sc->surface, ZI_NULL);
		zi_mem_free(sc);
		return (ZiSwapchainHandle){0};
	}

	return (ZiSwapchainHandle){.handler = sc};
}

static void zi_vulkan_swapchain_destroy(ZiSwapchainHandle handle) {
	if (!handle.handler) return;
	ZiVulkanSwapchain* sc = (ZiVulkanSwapchain*)handle.handler;

	zi_vulkan_swapchain_destroy_resources(sc);

	for (u32 i = 0; i < ZI_FRAMES_IN_FLIGHT; i++) {
		if (sc->image_available_semaphores[i]) vkDestroySemaphore(device, sc->image_available_semaphores[i], ZI_NULL);
		if (sc->render_finished_semaphores[i]) vkDestroySemaphore(device, sc->render_finished_semaphores[i], ZI_NULL);
	}

	if (sc->surface) {
		vkDestroySurfaceKHR(instance, sc->surface, ZI_NULL);
	}

	zi_mem_free(sc);
}

static void zi_vulkan_swapchain_resize(ZiSwapchainHandle handle, u32 width, u32 height) {
	if (!handle.handler) return;
	ZiVulkanSwapchain* sc = (ZiVulkanSwapchain*)handle.handler;

	sc->width = width;
	sc->height = height;

	zi_vulkan_swapchain_destroy_resources(sc);
	zi_vulkan_swapchain_create_resources(sc);
}

static u32 zi_vulkan_swapchain_get_texture_count(ZiSwapchainHandle handle) {
	if (!handle.handler) return 0;
	ZiVulkanSwapchain* sc = (ZiVulkanSwapchain*)handle.handler;
	return sc->image_count;
}

static ZiTextureHandle zi_vulkan_swapchain_get_texture(ZiSwapchainHandle handle, u32 index) {
	if (!handle.handler) return (ZiTextureHandle){0};
	ZiVulkanSwapchain* sc = (ZiVulkanSwapchain*)handle.handler;
	if (index >= sc->image_count) return (ZiTextureHandle){0};
	return (ZiTextureHandle){.handler = &sc->textures[index]};
}

static void zi_vulkan_swapchain_present(ZiSwapchainHandle handle) {
	if (!handle.handler) return;
	ZiVulkanSwapchain* sc = (ZiVulkanSwapchain*)handle.handler;

	// Acquire next image
	u32 image_index;
	VkResult result = vkAcquireNextImageKHR(device, sc->swapchain, U64_MAX,
		sc->image_available_semaphores[sc->current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// Swapchain needs to be recreated
		zi_vulkan_swapchain_destroy_resources(sc);
		zi_vulkan_swapchain_create_resources(sc);
		return;
	}

	sc->current_image_index = image_index;

	// Submit presentation
	VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &sc->render_finished_semaphores[sc->current_frame];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &sc->swapchain;
	present_info.pImageIndices = &sc->current_image_index;

	result = vkQueuePresentKHR(present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		zi_vulkan_swapchain_destroy_resources(sc);
		zi_vulkan_swapchain_create_resources(sc);
	}

	sc->current_frame = (sc->current_frame + 1) % ZI_FRAMES_IN_FLIGHT;
}

// Debug
static void zi_vulkan_set_object_name(void* handle, const char* name) {
	if (!debug_utils_enabled || !handle || !name) return;

	VkDebugUtilsObjectNameInfoEXT name_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
	name_info.pObjectName = name;
	// Note: This is a generic setter - callers should use type-specific wrappers
	// For now we assume it's a VkBuffer/VkImage handle
	name_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
	name_info.objectHandle = (u64)handle;

	vkSetDebugUtilsObjectNameEXT(device, &name_info);
}

static void zi_vulkan_cmd_begin_debug_label(ZiCommandBufferHandle cmd, const char* label) {
	if (!debug_utils_enabled || !cmd.handler || !label) return;

	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;

	VkDebugUtilsLabelEXT label_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
	label_info.pLabelName = label;
	label_info.color[0] = 1.0f;
	label_info.color[1] = 1.0f;
	label_info.color[2] = 1.0f;
	label_info.color[3] = 1.0f;

	vkCmdBeginDebugUtilsLabelEXT(vk_cmd->cmd, &label_info);
}

static void zi_vulkan_cmd_end_debug_label(ZiCommandBufferHandle cmd) {
	if (!debug_utils_enabled || !cmd.handler) return;

	ZiVulkanCommandBuffer* vk_cmd = (ZiVulkanCommandBuffer*)cmd.handler;
	vkCmdEndDebugUtilsLabelEXT(vk_cmd->cmd);
}

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

	// Framebuffer
	device->framebuffer_create = zi_vulkan_framebuffer_create;
	device->framebuffer_destroy = zi_vulkan_framebuffer_destroy;

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
