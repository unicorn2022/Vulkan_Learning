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
	vk::Pipeline graphicsPipeline = nullptr;		// ͼ����Ⱦ����
	vk::RenderPass renderPass = nullptr;			// ��Ⱦͨ��
	vk::PipelineLayout layout = nullptr;			// uniform ���岼��
	vk::DescriptorSetLayout setLayout = nullptr;	// uniform set��������

private:
	// ������Ⱦ����Layout
	vk::PipelineLayout createLayout();
	// ����ͼ����Ⱦ����
	vk::Pipeline createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
	// ������Ⱦͨ��
	vk::RenderPass createRenderPass();
	// ����uniform set��������
	vk::DescriptorSetLayout createSetLayout();

};
}