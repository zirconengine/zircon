#pragma once
#include "zi_common.h"


ZI_HANDLER(ZiBufferHandle);
ZI_HANDLER(ZiTextureHandle);
ZI_HANDLER(ZiTextureViewHandle);
ZI_HANDLER(ZiSamplerHandle);
ZI_HANDLER(ZiRenderPassHandle);
ZI_HANDLER(ZiCommandBufferHandle);
ZI_HANDLER(ZiSwapchainHandle);
ZI_HANDLER(ZiPipelineLayoutHandle);
ZI_HANDLER(ZiBindGroupLayoutHandle);

enum ZiGraphicsBackend_ {
	ZiGraphicsBackend_Vulkan = 1,
	ZiGraphicsBackend_Metal  = 2,
	ZiGraphicsBackend_D3D12  = 3,
	ZiGraphicsBackend_WebGPU = 4
};

typedef u32 ZiGraphicsBackend;

enum ZiFormat_ {
	ZiFormat_Undefined = 0,
	// 8-bit formats
	ZiFormat_R8_Unorm,
	ZiFormat_R8_Snorm,
	ZiFormat_R8_Uint,
	ZiFormat_R8_Sint,
	// 16-bit formats
	ZiFormat_R16_Unorm,
	ZiFormat_R16_Snorm,
	ZiFormat_R16_Uint,
	ZiFormat_R16_Sint,
	ZiFormat_R16_Float,
	ZiFormat_RG8_Unorm,
	ZiFormat_RG8_Snorm,
	ZiFormat_RG8_Uint,
	ZiFormat_RG8_Sint,
	// 32-bit formats
	ZiFormat_R32_Uint,
	ZiFormat_R32_Sint,
	ZiFormat_R32_Float,
	ZiFormat_RG16_Unorm,
	ZiFormat_RG16_Snorm,
	ZiFormat_RG16_Uint,
	ZiFormat_RG16_Sint,
	ZiFormat_RG16_Float,
	ZiFormat_RGBA8_Unorm,
	ZiFormat_RGBA8_Snorm,
	ZiFormat_RGBA8_Uint,
	ZiFormat_RGBA8_Sint,
	ZiFormat_RGBA8_Srgb,
	ZiFormat_BGRA8_Unorm,
	ZiFormat_BGRA8_Srgb,
	// 64-bit formats
	ZiFormat_RG32_Uint,
	ZiFormat_RG32_Sint,
	ZiFormat_RG32_Float,
	ZiFormat_RGBA16_Unorm,
	ZiFormat_RGBA16_Snorm,
	ZiFormat_RGBA16_Uint,
	ZiFormat_RGBA16_Sint,
	ZiFormat_RGBA16_Float,
	// 128-bit formats
	ZiFormat_RGBA32_Uint,
	ZiFormat_RGBA32_Sint,
	ZiFormat_RGBA32_Float,
	// Packed formats
	ZiFormat_RGB10A2_Unorm,
	ZiFormat_RGB10A2_Uint,
	ZiFormat_RG11B10_Float,
	ZiFormat_RGB9E5_Float,
	ZiFormat_B5G6R5_Unorm,
	ZiFormat_BGR5A1_Unorm,
	ZiFormat_BGRA4_Unorm,
	// Depth/stencil formats
	ZiFormat_D16_Unorm,
	ZiFormat_D24_Unorm_S8_Uint,
	ZiFormat_D32_Float,
	ZiFormat_D32_Float_S8_Uint,
	ZiFormat_S8_Uint,
	// BC compressed formats
	ZiFormat_BC1_Unorm,
	ZiFormat_BC1_Srgb,
	ZiFormat_BC2_Unorm,
	ZiFormat_BC2_Srgb,
	ZiFormat_BC3_Unorm,
	ZiFormat_BC3_Srgb,
	ZiFormat_BC4_Unorm,
	ZiFormat_BC4_Snorm,
	ZiFormat_BC5_Unorm,
	ZiFormat_BC5_Snorm,
	ZiFormat_BC6H_UFloat,
	ZiFormat_BC6H_SFloat,
	ZiFormat_BC7_Unorm,
	ZiFormat_BC7_Srgb,
	// ETC2 compressed formats
	ZiFormat_ETC2_RGB8_Unorm,
	ZiFormat_ETC2_RGB8_Srgb,
	ZiFormat_ETC2_RGB8A1_Unorm,
	ZiFormat_ETC2_RGB8A1_Srgb,
	ZiFormat_ETC2_RGBA8_Unorm,
	ZiFormat_ETC2_RGBA8_Srgb,
	ZiFormat_EAC_R11_Unorm,
	ZiFormat_EAC_R11_Snorm,
	ZiFormat_EAC_RG11_Unorm,
	ZiFormat_EAC_RG11_Snorm,
	// ASTC compressed formats
	ZiFormat_ASTC_4x4_Unorm,
	ZiFormat_ASTC_4x4_Srgb,
	ZiFormat_ASTC_5x4_Unorm,
	ZiFormat_ASTC_5x4_Srgb,
	ZiFormat_ASTC_5x5_Unorm,
	ZiFormat_ASTC_5x5_Srgb,
	ZiFormat_ASTC_6x5_Unorm,
	ZiFormat_ASTC_6x5_Srgb,
	ZiFormat_ASTC_6x6_Unorm,
	ZiFormat_ASTC_6x6_Srgb,
	ZiFormat_ASTC_8x5_Unorm,
	ZiFormat_ASTC_8x5_Srgb,
	ZiFormat_ASTC_8x6_Unorm,
	ZiFormat_ASTC_8x6_Srgb,
	ZiFormat_ASTC_8x8_Unorm,
	ZiFormat_ASTC_8x8_Srgb,
	ZiFormat_ASTC_10x5_Unorm,
	ZiFormat_ASTC_10x5_Srgb,
	ZiFormat_ASTC_10x6_Unorm,
	ZiFormat_ASTC_10x6_Srgb,
	ZiFormat_ASTC_10x8_Unorm,
	ZiFormat_ASTC_10x8_Srgb,
	ZiFormat_ASTC_10x10_Unorm,
	ZiFormat_ASTC_10x10_Srgb,
	ZiFormat_ASTC_12x10_Unorm,
	ZiFormat_ASTC_12x10_Srgb,
	ZiFormat_ASTC_12x12_Unorm,
	ZiFormat_ASTC_12x12_Srgb,
};

typedef u32 ZiFormat;

enum ZiIndexFormat_ {
	ZiIndexFormat_Uint16 = 0,
	ZiIndexFormat_Uint32,
};

typedef u32 ZiIndexFormat;

enum ZiBufferUsage_ {
	ZiBufferUsage_None    = 0,
	ZiBufferUsage_Vertex  = 1 << 0,
	ZiBufferUsage_Index   = 1 << 1,
	ZiBufferUsage_Uniform = 1 << 2,
	ZiBufferUsage_Storage = 1 << 3,
	ZiBufferUsage_CopySrc = 1 << 4,
	ZiBufferUsage_CopyDst = 1 << 5,
};

typedef u32 ZiBufferUsage;

enum ZiMemoryUsage_ {
	ZiMemoryUsage_GpuOnly  = 0,
	ZiMemoryUsage_CpuToGpu = 1,
	ZiMemoryUsage_GpuToCpu = 2,
};

typedef u32 ZiMemoryUsage;

typedef struct ZiBufferDesc {
	u64           size;
	ZiBufferUsage usage;
	ZiMemoryUsage memory;
} ZiBufferDesc;

enum ZiTextureDimension_ {
	ZiTextureDimension_1D = 0,
	ZiTextureDimension_2D,
	ZiTextureDimension_3D,
};

typedef u32 ZiTextureDimension;

enum ZiTextureUsage_ {
	ZiTextureUsage_None         = 0,
	ZiTextureUsage_Sampled      = 1 << 0,
	ZiTextureUsage_Storage      = 1 << 1,
	ZiTextureUsage_RenderTarget = 1 << 2,
	ZiTextureUsage_DepthStencil = 1 << 3,
	ZiTextureUsage_CopySrc      = 1 << 4,
	ZiTextureUsage_CopyDst      = 1 << 5,
};

typedef u32 ZiTextureUsage;

typedef struct ZiTextureDesc {
	ZiTextureDimension dimension;
	ZiFormat           format;
	u32                width;
	u32                height;
	u32                depth;
	u32                mip_levels;
	u32                array_layers;
	u32                sample_count;
	ZiTextureUsage     usage;
} ZiTextureDesc;

enum ZiTextureViewDimension_ {
	ZiTextureViewDimension_1D = 0,
	ZiTextureViewDimension_2D,
	ZiTextureViewDimension_2DArray,
	ZiTextureViewDimension_Cube,
	ZiTextureViewDimension_CubeArray,
	ZiTextureViewDimension_3D,
};

typedef u32 ZiTextureViewDimension;

enum ZiTextureAspect_ {
	ZiTextureAspect_All = 0,
	ZiTextureAspect_Color,
	ZiTextureAspect_Depth,
	ZiTextureAspect_Stencil,
};

typedef u32 ZiTextureAspect;

typedef struct ZiTextureViewDesc {
	ZiTextureHandle        texture;
	ZiTextureViewDimension dimension;
	ZiFormat               format;
	ZiTextureAspect        aspect;
	u32                    base_mip_level;
	u32                    mip_level_count;
	u32                    base_array_layer;
	u32                    array_layer_count;
} ZiTextureViewDesc;

enum ZiFilterMode_ {
	ZiFilterMode_Nearest = 0,
	ZiFilterMode_Linear,
};

typedef u32 ZiFilterMode;

enum ZiAddressMode_ {
	ZiAddressMode_Repeat = 0,
	ZiAddressMode_MirrorRepeat,
	ZiAddressMode_ClampToEdge,
	ZiAddressMode_ClampToBorder,
};

typedef u32 ZiAddressMode;

enum ZiCompareFunc_ {
	ZiCompareFunc_Never = 0,
	ZiCompareFunc_Less,
	ZiCompareFunc_Equal,
	ZiCompareFunc_LessEqual,
	ZiCompareFunc_Greater,
	ZiCompareFunc_NotEqual,
	ZiCompareFunc_GreaterEqual,
	ZiCompareFunc_Always,
};

typedef u32 ZiCompareFunc;

typedef struct ZiSamplerDesc {
	ZiFilterMode  min_filter;
	ZiFilterMode  mag_filter;
	ZiFilterMode  mipmap_filter;
	ZiAddressMode address_u;
	ZiAddressMode address_v;
	ZiAddressMode address_w;
	f32           min_lod;
	f32           max_lod;
	f32           max_anisotropy;
	ZiCompareFunc compare;
} ZiSamplerDesc;


enum ZiShaderStage_ {
	ZiShaderStage_Vertex   = 1 << 0,
	ZiShaderStage_Fragment = 1 << 1,
	ZiShaderStage_Compute  = 1 << 2,
};

typedef u32 ZiShaderStage;

typedef struct ZiShaderDesc {
	ZiShaderStage stage;
	ConstPtr      code;
	u64           code_size;
	ConstChr      entry_point;
} ZiShaderDesc;

ZI_HANDLER(ZiShaderHandle);

enum ZiPrimitiveTopology_ {
	ZiPrimitiveTopology_PointList = 0,
	ZiPrimitiveTopology_LineList,
	ZiPrimitiveTopology_LineStrip,
	ZiPrimitiveTopology_TriangleList,
	ZiPrimitiveTopology_TriangleStrip,
};

typedef u32 ZiPrimitiveTopology;

enum ZiCullMode_ {
	ZiCullMode_None = 0,
	ZiCullMode_Front,
	ZiCullMode_Back,
};

typedef u32 ZiCullMode;

enum ZiFrontFace_ {
	ZiFrontFace_CCW = 0,
	ZiFrontFace_CW,
};

typedef u32 ZiFrontFace;

enum ZiBlendFactor_ {
	ZiBlendFactor_Zero = 0,
	ZiBlendFactor_One,
	ZiBlendFactor_SrcColor,
	ZiBlendFactor_OneMinusSrcColor,
	ZiBlendFactor_SrcAlpha,
	ZiBlendFactor_OneMinusSrcAlpha,
	ZiBlendFactor_DstColor,
	ZiBlendFactor_OneMinusDstColor,
	ZiBlendFactor_DstAlpha,
	ZiBlendFactor_OneMinusDstAlpha,
};

typedef u32 ZiBlendFactor;

enum ZiBlendOp_ {
	ZiBlendOp_Add = 0,
	ZiBlendOp_Subtract,
	ZiBlendOp_ReverseSubtract,
	ZiBlendOp_Min,
	ZiBlendOp_Max,
};

typedef u32 ZiBlendOp;

enum ZiVertexInputRate_ {
	ZiVertexInputRate_Vertex = 0,
	ZiVertexInputRate_Instance,
};

typedef u32 ZiVertexInputRate;

typedef struct ZiVertexAttribute {
	u32      location;
	u32      binding;
	ZiFormat format;
	u32      offset;
} ZiVertexAttribute;

typedef struct ZiVertexBinding {
	u32               binding;
	u32               stride;
	ZiVertexInputRate input_rate;
} ZiVertexBinding;

typedef struct ZiBlendState {
	u8            blend_enable;
	ZiBlendFactor src_color;
	ZiBlendFactor dst_color;
	ZiBlendOp     color_op;
	ZiBlendFactor src_alpha;
	ZiBlendFactor dst_alpha;
	ZiBlendOp     alpha_op;
} ZiBlendState;

typedef struct ZiDepthStencilState {
	u8            depth_test_enable;
	u8            depth_write_enable;
	ZiCompareFunc depth_compare;
} ZiDepthStencilState;

typedef struct ZiRasterizerState {
	ZiCullMode  cull_mode;
	ZiFrontFace front_face;
	u8          wireframe;
} ZiRasterizerState;

typedef struct ZiPipelineLayoutDesc {
	ZiBindGroupLayoutHandle* bind_group_layouts;
	u32                      bind_group_layout_count;
} ZiPipelineLayoutDesc;

typedef struct ZiGraphicsPipelineDesc {
	ZiShaderHandle         vertex_shader;
	ZiShaderHandle         fragment_shader;
	ZiPipelineLayoutHandle layout;
	ZiVertexBinding*       vertex_bindings;
	u32                    vertex_binding_count;
	ZiVertexAttribute*     vertex_attributes;
	u32                    vertex_attribute_count;
	ZiPrimitiveTopology    topology;
	ZiRasterizerState      rasterizer;
	ZiDepthStencilState    depth_stencil;
	ZiBlendState           blend;
	ZiFormat               color_format;
	ZiFormat               depth_format;
} ZiGraphicsPipelineDesc;

typedef struct ZiComputePipelineDesc {
	ZiShaderHandle         compute_shader;
	ZiPipelineLayoutHandle layout;
} ZiComputePipelineDesc;

ZI_HANDLER(ZiPipelineHandle);

enum ZiLoadOp_ {
	ZiLoadOp_Load = 0,
	ZiLoadOp_Clear,
	ZiLoadOp_DontCare,
};

typedef u32 ZiLoadOp;

enum ZiStoreOp_ {
	ZiStoreOp_Store = 0,
	ZiStoreOp_DontCare,
};

typedef u32 ZiStoreOp;

typedef struct ZiColorAttachment {
	ZiTextureHandle texture;
	ZiLoadOp        load_op;
	ZiStoreOp       store_op;
	f32             clear_color[4];
} ZiColorAttachment;

typedef struct ZiDepthAttachment {
	ZiTextureHandle texture;
	ZiLoadOp        load_op;
	ZiStoreOp       store_op;
	f32             clear_depth;
	u8              clear_stencil;
} ZiDepthAttachment;

typedef struct ZiRenderPassDesc {
	ZiColorAttachment* color_attachments;
	u32                color_attachment_count;
	ZiDepthAttachment* depth_attachment;
} ZiRenderPassDesc;

enum ZiBindingType_ {
	ZiBindingType_UniformBuffer = 0,
	ZiBindingType_StorageBuffer,
	ZiBindingType_Sampler,
	ZiBindingType_SampledTexture,
	ZiBindingType_StorageTexture,
};

typedef u32 ZiBindingType;

typedef struct ZiBindGroupLayoutEntry {
	u32           binding;
	ZiShaderStage visibility;
	ZiBindingType type;
} ZiBindGroupLayoutEntry;

typedef struct ZiBindGroupLayoutDesc {
	ZiBindGroupLayoutEntry* entries;
	u32                     entry_count;
} ZiBindGroupLayoutDesc;

typedef struct ZiBindGroupEntry {
	u32             binding;
	ZiBufferHandle  buffer;
	u64             offset;
	u64             size;
	ZiTextureHandle texture;
	ZiSamplerHandle sampler;
} ZiBindGroupEntry;

typedef struct ZiBindGroupDesc {
	ZiBindGroupLayoutHandle layout;
	ZiBindGroupEntry*       entries;
	u32                     entry_count;
} ZiBindGroupDesc;

ZI_HANDLER(ZiBindGroupHandle);

enum ZiPresentMode_ {
	ZiPresentMode_Immediate = 0,
	ZiPresentMode_VSync,
	ZiPresentMode_Mailbox,
};

typedef u32 ZiPresentMode;

typedef struct ZiSwapchainDesc {
	VoidPtr  window_handle;
	ZiFormat format;
	u8       vsync;
} ZiSwapchainDesc;

typedef struct ZiDeviceLimits {
	u32 max_texture_dimension_1d;
	u32 max_texture_dimension_2d;
	u32 max_texture_dimension_3d;
	u32 max_texture_array_layers;
	u32 max_bind_groups;
	u32 max_bindings_per_bind_group;
	u32 max_uniform_buffer_size;
	u32 max_storage_buffer_size;
	u32 max_vertex_buffers;
	u32 max_vertex_attributes;
	u32 max_vertex_buffer_stride;
	u32 max_color_attachments;
	u32 max_compute_workgroup_size_x;
	u32 max_compute_workgroup_size_y;
	u32 max_compute_workgroup_size_z;
	u32 max_compute_workgroups_per_dimension;
	u32 max_push_constant_size;
} ZiDeviceLimits;

typedef struct ZiDeviceFeatures {
	ZiBool bindless_texture_supported;
	ZiBool bindless_sampler_supported;
	ZiBool bindless_buffer_supported;
	ZiBool multiview_enabled;
	ZiBool draw_indirect_count;
	ZiBool ray_tracing;
	ZiBool resolve_depth;
} ZiDeviceFeatures;

typedef struct ZiRenderDevice {
	void (*init)();
	void (*terminate)();
	void (*get_device_limits)(ZiDeviceLimits* limits);

	// Buffer
	ZiBufferHandle          (*buffer_create)(const ZiBufferDesc* desc);
	void                    (*buffer_destroy)(ZiBufferHandle handle);
	void                    (*buffer_write)(ZiBufferHandle handle, u64 offset, const void* data, u64 size);
	void*                   (*buffer_map)(ZiBufferHandle handle, u64 offset, u64 size);
	void                    (*buffer_unmap)(ZiBufferHandle handle);

	// Texture
	ZiTextureHandle         (*texture_create)(const ZiTextureDesc* desc);
	void                    (*texture_destroy)(ZiTextureHandle handle);

	// Texture View
	ZiTextureViewHandle     (*texture_view_create)(const ZiTextureViewDesc* desc);
	void                    (*texture_view_destroy)(ZiTextureViewHandle handle);

	// Sampler
	ZiSamplerHandle         (*sampler_create)(const ZiSamplerDesc* desc);
	void                    (*sampler_destroy)(ZiSamplerHandle handle);

	// Shader
	ZiShaderHandle          (*shader_create)(const ZiShaderDesc* desc);
	void                    (*shader_destroy)(ZiShaderHandle handle);

	// Pipeline Layout
	ZiPipelineLayoutHandle  (*pipeline_layout_create)(const ZiPipelineLayoutDesc* desc);
	void                    (*pipeline_layout_destroy)(ZiPipelineLayoutHandle handle);

	// Graphics Pipeline
	ZiPipelineHandle        (*graphics_pipeline_create)(const ZiGraphicsPipelineDesc* desc);
	void                    (*graphics_pipeline_destroy)(ZiPipelineHandle handle);

	// Compute Pipeline
	ZiPipelineHandle        (*compute_pipeline_create)(const ZiComputePipelineDesc* desc);
	void                    (*compute_pipeline_destroy)(ZiPipelineHandle handle);

	// Bind Group Layout
	ZiBindGroupLayoutHandle (*bind_group_layout_create)(const ZiBindGroupLayoutDesc* desc);
	void                    (*bind_group_layout_destroy)(ZiBindGroupLayoutHandle handle);

	// Bind Group
	ZiBindGroupHandle       (*bind_group_create)(const ZiBindGroupDesc* desc);
	void                    (*bind_group_destroy)(ZiBindGroupHandle handle);

	// Render Pass
	ZiRenderPassHandle      (*render_pass_create)(const ZiRenderPassDesc* desc);
	void                    (*render_pass_destroy)(ZiRenderPassHandle handle);

	// Command Buffer
	ZiCommandBufferHandle   (*command_buffer_create)();
	void                    (*command_buffer_destroy)(ZiCommandBufferHandle handle);
	void                    (*command_buffer_begin)(ZiCommandBufferHandle handle);
	void                    (*command_buffer_end)(ZiCommandBufferHandle handle);
	void                    (*command_buffer_submit)(ZiCommandBufferHandle handle);

	// Command Buffer - Render Pass
	void                    (*cmd_begin_render_pass)(ZiCommandBufferHandle cmd, const ZiRenderPassDesc* desc);
	void                    (*cmd_end_render_pass)(ZiCommandBufferHandle cmd);

	// Command Buffer - State
	void                    (*cmd_set_pipeline)(ZiCommandBufferHandle cmd, ZiPipelineHandle pipeline);
	void                    (*cmd_set_bind_group)(ZiCommandBufferHandle cmd, u32 index, ZiBindGroupHandle bind_group);
	void                    (*cmd_set_vertex_buffer)(ZiCommandBufferHandle cmd, u32 slot, ZiBufferHandle buffer, u64 offset);
	void                    (*cmd_set_index_buffer)(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, ZiIndexFormat format);
	void                    (*cmd_push_constants)(ZiCommandBufferHandle cmd, ZiShaderStage stages, u32 offset, u32 size, const void* data);
	void                    (*cmd_set_viewport)(ZiCommandBufferHandle cmd, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth);
	void                    (*cmd_set_scissor)(ZiCommandBufferHandle cmd, u32 x, u32 y, u32 width, u32 height);
	void                    (*cmd_set_blend_constant)(ZiCommandBufferHandle cmd, f32 color[4]);
	void                    (*cmd_set_stencil_reference)(ZiCommandBufferHandle cmd, u32 reference);

	// Command Buffer - Draw
	void                    (*cmd_draw)(ZiCommandBufferHandle cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
	void                    (*cmd_draw_indexed)(ZiCommandBufferHandle cmd, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance);
	void                    (*cmd_draw_indirect)(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride);
	void                    (*cmd_draw_indexed_indirect)(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride);

	// Command Buffer - Compute
	void                    (*cmd_dispatch)(ZiCommandBufferHandle cmd, u32 group_count_x, u32 group_count_y, u32 group_count_z);
	void                    (*cmd_dispatch_indirect)(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset);

	// Command Buffer - Copy
	void                    (*cmd_copy_buffer)(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiBufferHandle dst, u64 dst_offset, u64 size);
	void                    (*cmd_copy_texture)(ZiCommandBufferHandle cmd, ZiTextureHandle src, ZiTextureHandle dst);
	void                    (*cmd_copy_buffer_to_texture)(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiTextureHandle dst, u32 mip_level, u32 array_layer);
	void                    (*cmd_copy_texture_to_buffer)(ZiCommandBufferHandle cmd, ZiTextureHandle src, u32 mip_level, u32 array_layer, ZiBufferHandle dst, u64 dst_offset);

	// Swapchain
	ZiSwapchainHandle       (*swapchain_create)(const ZiSwapchainDesc* desc);
	void                    (*swapchain_destroy)(ZiSwapchainHandle handle);
	void                    (*swapchain_resize)(ZiSwapchainHandle handle, u32 width, u32 height);
	u32                     (*swapchain_get_texture_count)(ZiSwapchainHandle handle);
	ZiTextureHandle         (*swapchain_get_texture)(ZiSwapchainHandle handle, u32 index);
	void                    (*swapchain_present)(ZiSwapchainHandle handle);

	// Debug
	void                    (*set_object_name)(void* handle, const char* name);
	void                    (*cmd_begin_debug_label)(ZiCommandBufferHandle cmd, const char* label);
	void                    (*cmd_end_debug_label)(ZiCommandBufferHandle cmd);
} ZiRenderDevice;


void										zi_get_device_limits(ZiDeviceLimits* limits);

// Buffer
ZiBufferHandle          zi_buffer_create(const ZiBufferDesc* desc);
void                    zi_buffer_destroy(ZiBufferHandle handle);
void                    zi_buffer_write(ZiBufferHandle handle, u64 offset, const void* data, u64 size);
void*                   zi_buffer_map(ZiBufferHandle handle, u64 offset, u64 size);
void                    zi_buffer_unmap(ZiBufferHandle handle);

// Texture
ZiTextureHandle         zi_texture_create(const ZiTextureDesc* desc);
void                    zi_texture_destroy(ZiTextureHandle handle);

// Texture View
ZiTextureViewHandle     zi_texture_view_create(const ZiTextureViewDesc* desc);
void                    zi_texture_view_destroy(ZiTextureViewHandle handle);

// Sampler
ZiSamplerHandle         zi_sampler_create(const ZiSamplerDesc* desc);
void                    zi_sampler_destroy(ZiSamplerHandle handle);

// Shader
ZiShaderHandle          zi_shader_create(const ZiShaderDesc* desc);
void                    zi_shader_destroy(ZiShaderHandle handle);

// Pipeline Layout
ZiPipelineLayoutHandle  zi_pipeline_layout_create(const ZiPipelineLayoutDesc* desc);
void                    zi_pipeline_layout_destroy(ZiPipelineLayoutHandle handle);

// Graphics Pipeline
ZiPipelineHandle        zi_graphics_pipeline_create(const ZiGraphicsPipelineDesc* desc);
void                    zi_graphics_pipeline_destroy(ZiPipelineHandle handle);

// Compute Pipeline
ZiPipelineHandle        zi_compute_pipeline_create(const ZiComputePipelineDesc* desc);
void                    zi_compute_pipeline_destroy(ZiPipelineHandle handle);

// Bind Group Layout
ZiBindGroupLayoutHandle zi_bind_group_layout_create(const ZiBindGroupLayoutDesc* desc);
void                    zi_bind_group_layout_destroy(ZiBindGroupLayoutHandle handle);

// Bind Group
ZiBindGroupHandle       zi_bind_group_create(const ZiBindGroupDesc* desc);
void                    zi_bind_group_destroy(ZiBindGroupHandle handle);

// Render Pass
ZiRenderPassHandle      zi_render_pass_create(const ZiRenderPassDesc* desc);
void                    zi_render_pass_destroy(ZiRenderPassHandle handle);

// Command Buffer
ZiCommandBufferHandle   zi_command_buffer_create();
void                    zi_command_buffer_destroy(ZiCommandBufferHandle handle);
void                    zi_command_buffer_begin(ZiCommandBufferHandle handle);
void                    zi_command_buffer_end(ZiCommandBufferHandle handle);
void                    zi_command_buffer_submit(ZiCommandBufferHandle handle);

// Command Buffer - Render Pass
void                    zi_cmd_begin_render_pass(ZiCommandBufferHandle cmd, const ZiRenderPassDesc* desc);
void                    zi_cmd_end_render_pass(ZiCommandBufferHandle cmd);

// Command Buffer - State
void                    zi_cmd_set_pipeline(ZiCommandBufferHandle cmd, ZiPipelineHandle pipeline);
void                    zi_cmd_set_bind_group(ZiCommandBufferHandle cmd, u32 index, ZiBindGroupHandle bind_group);
void                    zi_cmd_set_vertex_buffer(ZiCommandBufferHandle cmd, u32 slot, ZiBufferHandle buffer, u64 offset);
void                    zi_cmd_set_index_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, ZiIndexFormat format);
void                    zi_cmd_push_constants(ZiCommandBufferHandle cmd, ZiShaderStage stages, u32 offset, u32 size, const void* data);
void                    zi_cmd_set_viewport(ZiCommandBufferHandle cmd, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth);
void                    zi_cmd_set_scissor(ZiCommandBufferHandle cmd, u32 x, u32 y, u32 width, u32 height);
void                    zi_cmd_set_blend_constant(ZiCommandBufferHandle cmd, f32 color[4]);
void                    zi_cmd_set_stencil_reference(ZiCommandBufferHandle cmd, u32 reference);

// Command Buffer - Draw
void                    zi_cmd_draw(ZiCommandBufferHandle cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
void                    zi_cmd_draw_indexed(ZiCommandBufferHandle cmd, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance);
void                    zi_cmd_draw_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride);
void                    zi_cmd_draw_indexed_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride);

// Command Buffer - Compute
void                    zi_cmd_dispatch(ZiCommandBufferHandle cmd, u32 group_count_x, u32 group_count_y, u32 group_count_z);
void                    zi_cmd_dispatch_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset);

// Command Buffer - Copy
void                    zi_cmd_copy_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiBufferHandle dst, u64 dst_offset, u64 size);
void                    zi_cmd_copy_texture(ZiCommandBufferHandle cmd, ZiTextureHandle src, ZiTextureHandle dst);
void                    zi_cmd_copy_buffer_to_texture(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiTextureHandle dst, u32 mip_level, u32 array_layer);
void                    zi_cmd_copy_texture_to_buffer(ZiCommandBufferHandle cmd, ZiTextureHandle src, u32 mip_level, u32 array_layer, ZiBufferHandle dst, u64 dst_offset);

// Swapchain
ZiSwapchainHandle       zi_swapchain_create(const ZiSwapchainDesc* desc);
void                    zi_swapchain_destroy(ZiSwapchainHandle handle);
void                    zi_swapchain_resize(ZiSwapchainHandle handle, u32 width, u32 height);
u32                     zi_swapchain_get_texture_count(ZiSwapchainHandle handle);
ZiTextureHandle         zi_swapchain_get_texture(ZiSwapchainHandle handle, u32 index);
void                    zi_swapchain_present(ZiSwapchainHandle handle);

// Debug
void                    zi_set_object_name(void* handle, const char* name);
void                    zi_cmd_begin_debug_label(ZiCommandBufferHandle cmd, const char* label);
void                    zi_cmd_end_debug_label(ZiCommandBufferHandle cmd);
