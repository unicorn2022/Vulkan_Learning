#pragma once

#include "vulkan/vulkan.hpp"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	RenderProcess();
	~RenderProcess();
	
	// 重新创建图形渲染管线
	void RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
	// 重新创建渲染通道
	void RecreateRenderPass();

public:
	vk::Pipeline graphicsPipeline = nullptr;	// 图形渲染管线
	vk::RenderPass renderPass = nullptr;		// 渲染通道
	vk::PipelineLayout layout = nullptr;		// uniform布局

private:
	// 创建渲染管线Layout
	vk::PipelineLayout createLayout();
	// 创建图形渲染管线
	vk::Pipeline createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
	// 创建渲染通道
	vk::RenderPass createRenderPass();

};
}