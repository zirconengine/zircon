#include "zi_graphics.h"

#include "zi_log.h"
#include "zi_platform.h"

void zi_graphics_init_vulkan(ZiRenderDevice* device);
void zi_graphics_init_webgpu(ZiRenderDevice* device);

static ZiRenderDevice device = {};

void zi_graphics_init(ZiGraphicsBackend backend) {

	backend = zi_platform_get_graphics_backend(backend);

	switch (backend) {
		case ZiGraphicsBackend_Vulkan:
			zi_graphics_init_vulkan(&device);
			break;
		case ZiGraphicsBackend_WebGPU:
			zi_graphics_init_webgpu(&device);
			break;
		case ZiGraphicsBackend_Metal:
		case ZiGraphicsBackend_D3D12:
			zi_log_error("not supported yet");
			return;
		default: ;
			return;
	}

	device.init();
}

void zi_graphics_terminate() {
	device.terminate();
}

void zi_get_device_limits(ZiDeviceLimits* limits) { device.get_device_limits(limits); }

// Buffer
ZiBufferHandle zi_buffer_create(const ZiBufferDesc* desc) { return device.buffer_create(desc); }
void zi_buffer_destroy(ZiBufferHandle handle) { device.buffer_destroy(handle); }
void zi_buffer_write(ZiBufferHandle handle, u64 offset, const void* data, u64 size) { device.buffer_write(handle, offset, data, size); }
void* zi_buffer_map(ZiBufferHandle handle, u64 offset, u64 size) { return device.buffer_map(handle, offset, size); }
void zi_buffer_unmap(ZiBufferHandle handle) { device.buffer_unmap(handle); }

// Texture
ZiTextureHandle zi_texture_create(const ZiTextureDesc* desc) { return device.texture_create(desc); }
void zi_texture_destroy(ZiTextureHandle handle) { device.texture_destroy(handle); }

// Texture View
ZiTextureViewHandle zi_texture_view_create(const ZiTextureViewDesc* desc) { return device.texture_view_create(desc); }
void zi_texture_view_destroy(ZiTextureViewHandle handle) { device.texture_view_destroy(handle); }

// Sampler
ZiSamplerHandle zi_sampler_create(const ZiSamplerDesc* desc) { return device.sampler_create(desc); }
void zi_sampler_destroy(ZiSamplerHandle handle) { device.sampler_destroy(handle); }

// Shader
ZiShaderHandle zi_shader_create(const ZiShaderDesc* desc) { return device.shader_create(desc); }
void zi_shader_destroy(ZiShaderHandle handle) { device.shader_destroy(handle); }

// Pipeline Layout
ZiPipelineLayoutHandle zi_pipeline_layout_create(const ZiPipelineLayoutDesc* desc) { return device.pipeline_layout_create(desc); }
void zi_pipeline_layout_destroy(ZiPipelineLayoutHandle handle) { device.pipeline_layout_destroy(handle); }

// Graphics Pipeline
ZiPipelineHandle zi_graphics_pipeline_create(const ZiGraphicsPipelineDesc* desc) { return device.graphics_pipeline_create(desc); }
void zi_graphics_pipeline_destroy(ZiPipelineHandle handle) { device.graphics_pipeline_destroy(handle); }

// Compute Pipeline
ZiPipelineHandle zi_compute_pipeline_create(const ZiComputePipelineDesc* desc) { return device.compute_pipeline_create(desc); }
void zi_compute_pipeline_destroy(ZiPipelineHandle handle) { device.compute_pipeline_destroy(handle); }

// Bind Group Layout
ZiBindGroupLayoutHandle zi_bind_group_layout_create(const ZiBindGroupLayoutDesc* desc) { return device.bind_group_layout_create(desc); }
void zi_bind_group_layout_destroy(ZiBindGroupLayoutHandle handle) { device.bind_group_layout_destroy(handle); }

// Bind Group
ZiBindGroupHandle zi_bind_group_create(const ZiBindGroupDesc* desc) { return device.bind_group_create(desc); }
void zi_bind_group_destroy(ZiBindGroupHandle handle) { device.bind_group_destroy(handle); }

// Render Pass
ZiRenderPassHandle zi_render_pass_create(const ZiRenderPassDesc* desc) { return device.render_pass_create(desc); }
void zi_render_pass_destroy(ZiRenderPassHandle handle) { device.render_pass_destroy(handle); }

// Command Buffer
ZiCommandBufferHandle zi_command_buffer_create() { return device.command_buffer_create(); }
void zi_command_buffer_destroy(ZiCommandBufferHandle handle) { device.command_buffer_destroy(handle); }
void zi_command_buffer_begin(ZiCommandBufferHandle handle) { device.command_buffer_begin(handle); }
void zi_command_buffer_end(ZiCommandBufferHandle handle) { device.command_buffer_end(handle); }
void zi_command_buffer_submit(ZiCommandBufferHandle handle) { device.command_buffer_submit(handle); }

// Command Buffer - Render Pass
void zi_cmd_begin_render_pass(ZiCommandBufferHandle cmd, const ZiRenderPassDesc* desc) { device.cmd_begin_render_pass(cmd, desc); }
void zi_cmd_end_render_pass(ZiCommandBufferHandle cmd) { device.cmd_end_render_pass(cmd); }

// Command Buffer - State
void zi_cmd_set_pipeline(ZiCommandBufferHandle cmd, ZiPipelineHandle pipeline) { device.cmd_set_pipeline(cmd, pipeline); }
void zi_cmd_set_bind_group(ZiCommandBufferHandle cmd, u32 index, ZiBindGroupHandle bind_group) { device.cmd_set_bind_group(cmd, index, bind_group); }
void zi_cmd_set_vertex_buffer(ZiCommandBufferHandle cmd, u32 slot, ZiBufferHandle buffer, u64 offset) { device.cmd_set_vertex_buffer(cmd, slot, buffer, offset); }
void zi_cmd_set_index_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, ZiIndexFormat format) { device.cmd_set_index_buffer(cmd, buffer, offset, format); }
void zi_cmd_push_constants(ZiCommandBufferHandle cmd, ZiShaderStage stages, u32 offset, u32 size, const void* data) { device.cmd_push_constants(cmd, stages, offset, size, data); }
void zi_cmd_set_viewport(ZiCommandBufferHandle cmd, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth) { device.cmd_set_viewport(cmd, x, y, width, height, min_depth, max_depth); }
void zi_cmd_set_scissor(ZiCommandBufferHandle cmd, u32 x, u32 y, u32 width, u32 height) { device.cmd_set_scissor(cmd, x, y, width, height); }
void zi_cmd_set_blend_constant(ZiCommandBufferHandle cmd, f32 color[4]) { device.cmd_set_blend_constant(cmd, color); }
void zi_cmd_set_stencil_reference(ZiCommandBufferHandle cmd, u32 reference) { device.cmd_set_stencil_reference(cmd, reference); }

// Command Buffer - Draw
void zi_cmd_draw(ZiCommandBufferHandle cmd, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) { device.cmd_draw(cmd, vertex_count, instance_count, first_vertex, first_instance); }
void zi_cmd_draw_indexed(ZiCommandBufferHandle cmd, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) { device.cmd_draw_indexed(cmd, index_count, instance_count, first_index, vertex_offset, first_instance); }
void zi_cmd_draw_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) { device.cmd_draw_indirect(cmd, buffer, offset, draw_count, stride); }
void zi_cmd_draw_indexed_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset, u32 draw_count, u32 stride) { device.cmd_draw_indexed_indirect(cmd, buffer, offset, draw_count, stride); }

// Command Buffer - Compute
void zi_cmd_dispatch(ZiCommandBufferHandle cmd, u32 group_count_x, u32 group_count_y, u32 group_count_z) { device.cmd_dispatch(cmd, group_count_x, group_count_y, group_count_z); }
void zi_cmd_dispatch_indirect(ZiCommandBufferHandle cmd, ZiBufferHandle buffer, u64 offset) { device.cmd_dispatch_indirect(cmd, buffer, offset); }

// Command Buffer - Copy
void zi_cmd_copy_buffer(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiBufferHandle dst, u64 dst_offset, u64 size) { device.cmd_copy_buffer(cmd, src, src_offset, dst, dst_offset, size); }
void zi_cmd_copy_texture(ZiCommandBufferHandle cmd, ZiTextureHandle src, ZiTextureHandle dst) { device.cmd_copy_texture(cmd, src, dst); }
void zi_cmd_copy_buffer_to_texture(ZiCommandBufferHandle cmd, ZiBufferHandle src, u64 src_offset, ZiTextureHandle dst, u32 mip_level, u32 array_layer) { device.cmd_copy_buffer_to_texture(cmd, src, src_offset, dst, mip_level, array_layer); }
void zi_cmd_copy_texture_to_buffer(ZiCommandBufferHandle cmd, ZiTextureHandle src, u32 mip_level, u32 array_layer, ZiBufferHandle dst, u64 dst_offset) { device.cmd_copy_texture_to_buffer(cmd, src, mip_level, array_layer, dst, dst_offset); }

// Swapchain
ZiSwapchainHandle zi_swapchain_create(const ZiSwapchainDesc* desc) { return device.swapchain_create(desc); }
void zi_swapchain_destroy(ZiSwapchainHandle handle) { device.swapchain_destroy(handle); }
void zi_swapchain_resize(ZiSwapchainHandle handle, u32 width, u32 height) { device.swapchain_resize(handle, width, height); }
u32 zi_swapchain_get_texture_count(ZiSwapchainHandle handle) { return device.swapchain_get_texture_count(handle); }
ZiTextureHandle zi_swapchain_get_texture(ZiSwapchainHandle handle, u32 index) { return device.swapchain_get_texture(handle, index); }
void zi_swapchain_present(ZiSwapchainHandle handle) { device.swapchain_present(handle); }

// Debug
void zi_set_object_name(void* handle, const char* name) { device.set_object_name(handle, name); }
void zi_cmd_begin_debug_label(ZiCommandBufferHandle cmd, const char* label) { device.cmd_begin_debug_label(cmd, label); }
void zi_cmd_end_debug_label(ZiCommandBufferHandle cmd) { device.cmd_end_debug_label(cmd); }
