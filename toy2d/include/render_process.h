#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.h"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	vk::Pipeline graphicsPipelineWithTriangleTopology = nullptr;	// (三角形图元)图形渲染管线
	vk::Pipeline graphicsPipelineWithLineTopology = nullptr;		// (线段图元)图形渲染管线
	vk::RenderPass renderPass = nullptr;			// 渲染通道
	vk::PipelineLayout layout = nullptr;			// uniform 整体布局
	
public:
	RenderProcess();
	~RenderProcess();
	
	// 创建图形渲染管线
	void CreateGraphicsPipeline(const Shader& shader);
	// 创建渲染通道
	void CreateRenderPass();

private:
	vk::PipelineCache pipelineCache_ = nullptr;	// 管线缓存

	// 创建渲染管线Layout
	vk::PipelineLayout createLayout();
	// 创建图形渲染管线
	vk::Pipeline createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology);
	// 创建渲染通道
	vk::RenderPass createRenderPass();
	// 创建管线缓存
	vk::PipelineCache createPipelineCache();
};
}