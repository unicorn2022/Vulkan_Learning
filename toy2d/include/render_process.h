#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.h"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	vk::Pipeline graphicsPipeline = nullptr;		// 图形渲染管线
	vk::RenderPass renderPass = nullptr;			// 渲染通道
	vk::PipelineLayout layout = nullptr;			// uniform 整体布局
	
public:
	RenderProcess();
	~RenderProcess();
	
	// 重新创建图形渲染管线
	void RecreateGraphicsPipeline(const Shader& shader);
	// 重新创建渲染通道
	void RecreateRenderPass();

private:
	// 创建渲染管线Layout
	vk::PipelineLayout createLayout();
	// 创建图形渲染管线
	vk::Pipeline createGraphicsPipeline(const Shader& shader);
	vk::RenderPass createRenderPass();
};
}