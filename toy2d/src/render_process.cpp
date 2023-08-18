#include "render_process.h"
#include "shader.h"
#include "context.h"
#include "swapchain.h"

namespace toy2d {

void RenderProcess::InitLayout() {
	vk::PipelineLayoutCreateInfo createInfo;
	layout = Context::GetInstance().device.createPipelineLayout(createInfo);
}

void RenderProcess::InitRenderPass() {
	vk::RenderPassCreateInfo createInfo;
	/* �������� */
	vk::AttachmentDescription attachDesc;
	attachDesc.setFormat(Context::GetInstance().swapchain->info.format.format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
		.setLoadOp(vk::AttachmentLoadOp::eClear)			// ��ɫ����: ����ʱ���
		.setStoreOp(vk::AttachmentStoreOp::eStore)			// ��ɫ����: �洢ʱ����	
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)	// ģ�建��
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)// ģ�建��
		.setSamples(vk::SampleCountFlagBits::e1);			// ��������
	createInfo.setAttachments(attachDesc);

	/* ��ͨ������ */
	vk::AttachmentReference reference;
	reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal) // ͼ�񲼾�
		.setAttachment(0); // ��������

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) // ������һ����ˮ����
		.setColorAttachments(reference); // ��ɫ����
	createInfo.setSubpasses(subpass);
	
	/* ��ͨ������ */
	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)	// ��ִ�е���ͨ��: �ⲿͨ��
		.setDstSubpass(0)							// ��ִ�е���ͨ��: 0����ͨ��
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite) // ��ͨ���ķ���Ȩ��
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)		
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // ��ͨ��������ͼ���Ӧ�ó���
	createInfo.setDependencies(dependency);

	/* ������Ⱦ���� */
	renderPass = Context::GetInstance().device.createRenderPass(createInfo);
}

void RenderProcess::InitPipeline(int width, int height) {
	/* ��ˮ������ */
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
	rastInfo.setRasterizerDiscardEnable(false)		// �Ƿ���Թ�դ�����
		.setCullMode(vk::CullModeFlagBits::eBack)	// �޳�����
		.setFrontFace(vk::FrontFace::eClockwise)	// �������淽��: ˳ʱ��
		.setPolygonMode(vk::PolygonMode::eFill)		// ��������ģʽ: ���
		.setLineWidth(1.0f);						// �߿�
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
		.setAttachments(attachs);	// ��ɫ��ϸ���
	createInfo.setPColorBlendState(&blend);

	// 9. render pass & layout
	createInfo.setRenderPass(renderPass)
		.setLayout(layout);


	/* ���� pipeline */
	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("create graphics pipeline failed");
	}
	pipeline = result.value;
}

RenderProcess::~RenderProcess() {
	auto& device = Context::GetInstance().device;
	device.destroyRenderPass(renderPass);
	device.destroyPipelineLayout(layout);
	device.destroyPipeline(pipeline);
}
}