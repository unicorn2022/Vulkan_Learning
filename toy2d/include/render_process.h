#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.h"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	vk::Pipeline graphicsPipeline = nullptr;		// ͼ����Ⱦ����
	vk::RenderPass renderPass = nullptr;			// ��Ⱦͨ��
	vk::PipelineLayout layout = nullptr;			// uniform ���岼��
	
public:
	RenderProcess();
	~RenderProcess();
	
	// ����ͼ����Ⱦ����
	void CreateGraphicsPipeline(const Shader& shader);
	// ������Ⱦͨ��
	void CreateRenderPass();

private:
	// ������Ⱦ����Layout
	vk::PipelineLayout createLayout();
	// ����ͼ����Ⱦ����
	vk::Pipeline createGraphicsPipeline(const Shader& shader);
	vk::RenderPass createRenderPass();
};
}