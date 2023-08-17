#include "render_process.h"
#include "shader.h"
#include "context.h"

namespace toy2d {
void RenderProcess::InitPipeline(int width, int height) {
	vk::GraphicsPipelineCreateInfo createInfo;

	// 1. ��������
	vk::PipelineVertexInputStateCreateInfo inputState;
	createInfo.setPVertexInputState(&inputState);

	// 2. ����ۼ�
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList); // ����ľۼ���ʽ: 123, 456, 789
	createInfo.setPInputAssemblyState(&inputAssembly);

	// 3. Shader
	auto stages = Shader::GetInstance().GetStage();
	createInfo.setStages(stages);

	// 4. �ӿڱ任
	// 4.1 �ӿ�
	vk::Viewport viewport(0, 0, width, height, 0, 1);
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.setViewports(viewport);
	// 4.2 ���л�
	vk::Rect2D rect({ 0, 0 }, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
	viewportState.setScissors(rect);
	createInfo.setPViewportState(&viewportState);

	// 5. ��դ��
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)			// �Ƿ���Թ�դ�����
		.setCullMode(vk::CullModeFlagBits::eBack)		// �޳�����
		.setFrontFace(vk::FrontFace::eCounterClockwise) // �������淽��: ��ʱ��
		.setPolygonMode(vk::PolygonMode::eFill)			// ��������ģʽ: ���
		.setLineWidth(1.0f)								// �߿�
		.setDepthClampEnable(false);					// ����޳�
	createInfo.setPRasterizationState(&rastInfo);

	// 6. ���ز���
	vk::PipelineMultisampleStateCreateInfo multiSample;
	multiSample.setSampleShadingEnable(false) // �Ƿ����ö��ز���
		.setRasterizationSamples(vk::SampleCountFlagBits::e1); // ��������
	createInfo.setPMultisampleState(&multiSample);

	// 7. ����: ��Ȳ���, ģ�����

	// 8. ��ɫ���
	vk::PipelineColorBlendAttachmentState attachs;
	attachs.setBlendEnable(false)	// �Ƿ����û�ϸ���
		.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR); // ��ɫд������
	vk::PipelineColorBlendStateCreateInfo blend;
	blend.setLogicOpEnable(false)	// �Ƿ������߼�����
		.setPAttachments(&attachs); // ��ɫ��ϸ���
	createInfo.setPColorBlendState(&blend);

	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("create graphics pipeline failed");
	}
	pipeline = result.value;
}

void RenderProcess::DestoryPipline() {
	Context::GetInstance().device.destroyPipeline(pipeline);
}
}