#pragma once
#include "zi_common.h"


ZI_HANDLER(ZiBufferHandle);
ZI_HANDLER(ZiTextureHandle);
ZI_HANDLER(ZiSamplerHandle);
ZI_HANDLER(ZiRenderPassHandle);
ZI_HANDLER(ZiCommandBufferHandle);
ZI_HANDLER(ZiSwapchainHandle);

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
	ZiFormat_R16_Uint,
	ZiFormat_R16_Sint,
	ZiFormat_R16_Float,
	ZiFormat_RG8_Unorm,
	ZiFormat_RG8_Snorm,
	// 32-bit formats
	ZiFormat_R32_Uint,
	ZiFormat_R32_Sint,
	ZiFormat_R32_Float,
	ZiFormat_RG16_Uint,
	ZiFormat_RG16_Float,
	ZiFormat_RGBA8_Unorm,
	ZiFormat_RGBA8_Snorm,
	ZiFormat_RGBA8_Srgb,
	ZiFormat_BGRA8_Unorm,
	ZiFormat_BGRA8_Srgb,
	// 64-bit formats
	ZiFormat_RG32_Uint,
	ZiFormat_RG32_Float,
	ZiFormat_RGBA16_Uint,
	ZiFormat_RGBA16_Float,
	// 128-bit formats
	ZiFormat_RGBA32_Uint,
	ZiFormat_RGBA32_Float,
	// Depth/stencil formats
	ZiFormat_D16_Unorm,
	ZiFormat_D24_Unorm_S8_Uint,
	ZiFormat_D32_Float,
	ZiFormat_D32_Float_S8_Uint,
};

typedef u32 ZiFormat;

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

typedef struct ZiBufferDesc {
	u64           size;
	ZiBufferUsage usage;
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
	ZiTextureUsage     usage;
} ZiTextureDesc;

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

typedef struct ZiSamplerDesc {
	ZiFilterMode  min_filter;
	ZiFilterMode  mag_filter;
	ZiFilterMode  mipmap_filter;
	ZiAddressMode address_u;
	ZiAddressMode address_v;
	ZiAddressMode address_w;
	f32           min_lod;
	f32           max_lod;
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

typedef struct ZiPipelineDesc {
	ZiShaderHandle      vertex_shader;
	ZiShaderHandle      fragment_shader;
	ZiVertexBinding*    vertex_bindings;
	u32                 vertex_binding_count;
	ZiVertexAttribute*  vertex_attributes;
	u32                 vertex_attribute_count;
	ZiPrimitiveTopology topology;
	ZiRasterizerState   rasterizer;
	ZiDepthStencilState depth_stencil;
	ZiBlendState        blend;
	ZiFormat            color_format;
	ZiFormat            depth_format;
} ZiPipelineDesc;

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

ZI_HANDLER(ZiBindGroupLayoutHandle);

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
	VoidPtr       window_handle;
	u32           width;
	u32           height;
	ZiFormat      format;
	ZiPresentMode present_mode;
} ZiSwapchainDesc;

typedef struct ZiRenderDevice {
	void (*init)();
	void (*terminate)();
} ZiRenderDevice;

void zi_graphics_init(ZiGraphicsBackend backend);
void zi_graphics_terminate();
