#pragma once

#include "vulkan/vulkan.hpp"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	RenderProcess();
	~RenderProcess();
	
	// ���´���ͼ����Ⱦ����
	void RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
	// ���´�����Ⱦͨ��
	void RecreateRenderPass();

public:
	vk::Pipeline graphicsPipeline = nullptr;	// ͼ����Ⱦ����
	vk::RenderPass renderPass = nullptr;		// ��Ⱦͨ��
	vk::PipelineLayout layout = nullptr;		// uniform����

private:
	// ������Ⱦ����Layout
	vk::PipelineLayout createLayout();
	// ����ͼ����Ⱦ����
	vk::Pipeline createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
	// ������Ⱦͨ��
	vk::RenderPass createRenderPass();

};
}