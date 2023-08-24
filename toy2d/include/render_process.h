#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.h"
#include <fstream>

namespace toy2d {

class RenderProcess final {
public:
	vk::Pipeline graphicsPipelineWithTriangleTopology = nullptr;	// (������ͼԪ)ͼ����Ⱦ����
	vk::Pipeline graphicsPipelineWithLineTopology = nullptr;		// (�߶�ͼԪ)ͼ����Ⱦ����
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
	vk::PipelineCache pipelineCache_ = nullptr;	// ���߻���

	// ������Ⱦ����Layout
	vk::PipelineLayout createLayout();
	// ����ͼ����Ⱦ����
	vk::Pipeline createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology topology);
	// ������Ⱦͨ��
	vk::RenderPass createRenderPass();
	// �������߻���
	vk::PipelineCache createPipelineCache();
};
}