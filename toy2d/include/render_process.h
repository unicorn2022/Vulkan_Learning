#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class RenderProcess final {
public:
	void InitLayout();
	void InitRenderPass();
	void InitPipeline(int width, int height);

	~RenderProcess();

public:
	vk::Pipeline pipeline;		// 渲染管线
	vk::PipelineLayout layout;	// uniform布局
	vk::RenderPass renderPass;	// 渲染通道
};
}