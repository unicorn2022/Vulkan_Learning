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
	vk::Pipeline pipeline;		// ��Ⱦ����
	vk::PipelineLayout layout;	// uniform����
	vk::RenderPass renderPass;	// ��Ⱦͨ��
};
}